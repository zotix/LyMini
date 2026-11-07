#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <LittleFS.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "SystemSettings.h"
#include "PowerManager.h"

class IdleScene {
private:
  uint8_t  _currentScreen; // 0: Clock, 1: Weather, 2: Photo, 3: Video
  uint32_t _screenTimer;
  uint32_t _liveHoldUntilMs;
  bool     _liveBufferActive;

  // Weather state
  float    _tempC;
  uint8_t  _humidity;
  uint32_t _lastWeatherFetch;

  // Photo list (/photos/*.rgb)
  char     _photoFiles[16][40];
  uint8_t  _photoCount;
  uint8_t  _curPhotoIdx;
  bool     _photoLoaded;

  // Video list (/videos/*.mvid)
  char     _videoFiles[10][40];
  uint8_t  _videoCount;
  uint8_t  _curVideoIdx;

  // Active .mvid stream state
  File     _videoFile;
  uint8_t  _vidFormat;     // 0 = RGB565, 1 = RGB332
  uint8_t  _vidFps;
  uint8_t  _vidFrameCount;
  uint8_t  _vidCurFrame;
  uint32_t _lastVidFrameMs;

  static inline uint16_t expandRgb332(uint8_t c) {
    uint8_t r3 = (c >> 5) & 0x07;
    uint8_t g3 = (c >> 2) & 0x07;
    uint8_t b2 = c & 0x03;
    uint16_t r5 = (r3 << 2) | (r3 >> 1);
    uint16_t g6 = (g3 << 3) | g3;
    uint16_t b5 = (b2 << 3) | (b2 << 1) | (b2 >> 1);
    return (r5 << 11) | (g6 << 5) | b5;
  }

  bool isScreenEnabled(uint8_t scr) const {
    uint8_t mask = gSettings.data.idle_enabled_mask;
    if (mask == 0) mask = IDLE_MASK_ALL;
    return (mask & (1 << scr)) != 0;
  }

  void pickNextScreen(int8_t dir = 1) {
    _liveBufferActive = false;
    _photoLoaded = false;

    uint8_t enabledList[4];
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < 4; ++i) {
      if (isScreenEnabled(i)) enabledList[cnt++] = i;
    }
    if (cnt == 0) {
      _currentScreen = 0;
      return;
    }

    if (gSettings.data.idle_random_mode && dir == 0) {
      _currentScreen = enabledList[random(0, cnt)];
    } else {
      int curPos = 0;
      for (uint8_t i = 0; i < cnt; ++i) {
        if (enabledList[i] == _currentScreen) { curPos = i; break; }
      }
      curPos = (curPos + dir + cnt) % cnt;
      _currentScreen = enabledList[curPos];
    }

    if (_currentScreen == 2 && _photoCount > 0) {
      if (gSettings.data.idle_random_mode) {
        _curPhotoIdx = random(0, _photoCount);
      } else {
        _curPhotoIdx = (_curPhotoIdx + 1) % _photoCount;
      }
    } else if (_currentScreen == 3 && _videoCount > 0) {
      if (gSettings.data.idle_random_mode) {
        _curVideoIdx = random(0, _videoCount);
      } else {
        _curVideoIdx = (_curVideoIdx + 1) % _videoCount;
      }
      openCurrentVideo();
    }
  }

  void openCurrentVideo() {
    if (_videoFile) _videoFile.close();
    if (_videoCount == 0) return;

    _videoFile = LittleFS.open(_videoFiles[_curVideoIdx], "r");
    if (!_videoFile || _videoFile.size() < 8) {
      if (_videoFile) _videoFile.close();
      return;
    }

    uint8_t hdr[8];
    _videoFile.read(hdr, 8);
    if (hdr[0] == 'M' && hdr[1] == 'V' && hdr[2] == '1') {
      _vidFormat     = hdr[3];
      _vidFps        = (hdr[6] > 0 && hdr[6] <= 30) ? hdr[6] : 12;
      _vidFrameCount = (hdr[7] > 0) ? hdr[7] : 1;
      _vidCurFrame   = 0;
      _lastVidFrameMs= 0;
    } else {
      _videoFile.close();
    }
  }

public:
  IdleScene() : _currentScreen(0), _screenTimer(0), _liveHoldUntilMs(0), _liveBufferActive(false),
                _tempC(28.5f), _humidity(65), _lastWeatherFetch(0),
                _photoCount(0), _curPhotoIdx(0), _photoLoaded(false),
                _videoCount(0), _curVideoIdx(0), _vidFormat(1), _vidFps(12),
                _vidFrameCount(1), _vidCurFrame(0), _lastVidFrameMs(0) {}

  void scanMediaFiles() {
    _photoCount = 0;
    _videoCount = 0;
    if (!LittleFS.begin(true)) return;

    File pdir = LittleFS.open("/photos");
    if (pdir && pdir.isDirectory()) {
      File file = pdir.openNextFile();
      while (file && _photoCount < 16) {
        String name = file.name();
        if (!file.isDirectory() && name.endsWith(".rgb") && gSettings.isMediaEnabled(name.c_str())) {
          snprintf(_photoFiles[_photoCount], sizeof(_photoFiles[0]), "/photos/%s", name.c_str());
          _photoCount++;
        }
        file = pdir.openNextFile();
      }
    }

    File vdir = LittleFS.open("/videos");
    if (vdir && vdir.isDirectory()) {
      File file = vdir.openNextFile();
      while (file && _videoCount < 10) {
        String name = file.name();
        if (!file.isDirectory() && name.endsWith(".mvid") && gSettings.isMediaEnabled(name.c_str())) {
          snprintf(_videoFiles[_videoCount], sizeof(_videoFiles[0]), "/videos/%s", name.c_str());
          _videoCount++;
        }
        file = vdir.openNextFile();
      }
    }

    if (_curPhotoIdx >= _photoCount) _curPhotoIdx = 0;
    if (_curVideoIdx >= _videoCount) _curVideoIdx = 0;
  }

  void init() {
    _screenTimer = millis();
    _liveBufferActive = false;
    _photoLoaded = false;
    scanMediaFiles();
    fetchWeatherIfNeeded();
    pickNextScreen(0);
  }

  // Triggered from Web Portal "Show on ESP" button
  void showSpecificMedia(const char* fullPath) {
    if (!fullPath) return;
    scanMediaFiles();
    _screenTimer = millis();
    _liveHoldUntilMs = millis() + 30000UL;
    _liveBufferActive = false;

    String p(fullPath);
    if (p.endsWith(".rgb")) {
      // Ensure it is in slot 0 or find it
      int found = -1;
      for (int i = 0; i < _photoCount; ++i) {
        if (strcmp(_photoFiles[i], fullPath) == 0) { found = i; break; }
      }
      if (found < 0 && _photoCount < 16) {
        strncpy(_photoFiles[_photoCount], fullPath, 39);
        found = _photoCount++;
      }
      if (found >= 0) {
        _curPhotoIdx = found;
        _currentScreen = 2;
        _photoLoaded = false;
      }
    } else if (p.endsWith(".mvid")) {
      int found = -1;
      for (int i = 0; i < _videoCount; ++i) {
        if (strcmp(_videoFiles[i], fullPath) == 0) { found = i; break; }
      }
      if (found < 0 && _videoCount < 10) {
        strncpy(_videoFiles[_videoCount], fullPath, 39);
        found = _videoCount++;
      }
      if (found >= 0) {
        _curVideoIdx = found;
        _currentScreen = 3;
        openCurrentVideo();
      }
    }
  }

  // Triggered when Web Portal streams a Live Frame directly into DisplayCanvas buffer
  void activateLiveViewBuffer() {
    _liveBufferActive = true;
    _liveHoldUntilMs = millis() + 15000UL;
    _screenTimer = millis();
  }

  void fetchWeatherIfNeeded() {
    if (!gPower.isWifiEnabled() || WiFi.status() != WL_CONNECTED) return;
    if (_lastWeatherFetch > 0 && millis() - _lastWeatherFetch < 600000UL) return;

    _lastWeatherFetch = millis();
    HTTPClient http;
    char url[140];
    snprintf(url, sizeof(url),
             "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m",
             gSettings.data.latitude, gSettings.data.longitude);

    http.setTimeout(2500);
    http.begin(url);
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      int tempIdx = payload.indexOf("\"temperature_2m\":");
      if (tempIdx > 0) _tempC = payload.substring(tempIdx + 17).toFloat();
      int humIdx = payload.indexOf("\"relative_humidity_2m\":");
      if (humIdx > 0) _humidity = payload.substring(humIdx + 23).toInt();
    }
    http.end();
  }

  // Returns true if user pressed BOOT to wake up back to Menu/Game
  bool update(const InputState& input, float dt) {
    if (input.okShortReleased || input.menuHold) {
      if (_videoFile) _videoFile.close();
      _liveBufferActive = false;
      return true;
    }

    // Manual Left / Right switching between Idle screens or photos/videos
    if (input.leftPressed) {
      pickNextScreen(-1);
      _screenTimer = millis();
    } else if (input.rightPressed) {
      pickNextScreen(1);
      _screenTimer = millis();
    }

    // Auto-cycle idle screens (unless held by Web Live View)
    uint32_t cycleMs = (uint32_t)max((uint16_t)5, gSettings.data.idle_cycle_sec) * 1000UL;
    if (millis() > _liveHoldUntilMs && (millis() - _screenTimer >= cycleMs)) {
      _screenTimer = millis();
      pickNextScreen(0);
    }

    fetchWeatherIfNeeded();
    return false;
  }

  void render(DisplayCanvas& gfx) {
    // If Web Live View wrote directly to the framebuffer, keep it intact!
    if (_liveBufferActive && millis() <= _liveHoldUntilMs) {
      return;
    }
    _liveBufferActive = false;

    if (_currentScreen == 0) {
      renderClock(gfx);
    } else if (_currentScreen == 1) {
      renderWeather(gfx);
    } else if (_currentScreen == 2) {
      renderPhoto(gfx);
    } else {
      renderVideo(gfx);
    }
  }

  void renderClock(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 12, 24));

    // Top Header
    gfx.fillRect(0, 0, SCREEN_WIDTH, 15, Color565::DeepBlue);
    gfx.drawFastHLine(0, 15, SCREEN_WIDTH, Color565::NeonCyan);
    gfx.drawCenteredText(4, "CYBER CLOCK", Color565::Cyan);

    char timeStr[16];
    char dateStr[24];
    if (gPower.isTimeSynced()) {
      time_t now = time(nullptr);
      struct tm timeinfo;
      localtime_r(&now, &timeinfo);
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
      const char* days[]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
      snprintf(dateStr, sizeof(dateStr), "%s, %d %s %d",
               days[timeinfo.tm_wday % 7], timeinfo.tm_mday, months[timeinfo.tm_mon % 12], timeinfo.tm_year + 1900);
    } else {
      uint32_t s = millis() / 1000;
      snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", (s / 3600) % 24, (s / 60) % 60, s % 60);
      snprintf(dateStr, sizeof(dateStr), "Syncing via Wi-Fi...");
    }

    // Big Digital Clock Card
    gfx.fillRoundRect(8, 26, 112, 34, 4, Color565::rgb(20, 26, 48));
    gfx.drawRoundRect(8, 26, 112, 34, 4, Color565::NeonCyan);
    gfx.drawCenteredText(36, timeStr, Color565::White, 2);

    gfx.drawCenteredText(68, dateStr, Color565::Gold);

    // Status: Wi-Fi & Power Save info
    if (!gPower.isWifiEnabled()) {
      gfx.drawCenteredText(86, "ECO MODE (WiFi OFF)", Color565::Lime);
      char bStr[24];
      snprintf(bStr, sizeof(bStr), "CPU %uMHz | BATT %u%%", gPower.getCpuMhz(), gPower.getBatteryPercent());
      gfx.drawCenteredText(98, bStr, Color565::SkyBlue);
    } else if (WiFi.status() == WL_CONNECTED) {
      gfx.drawCenteredText(86, WiFi.localIP().toString().c_str(), Color565::Lime);
      gfx.drawCenteredText(98, "http://mini.local", Color565::NeonCyan);
    } else {
      gfx.drawCenteredText(86, "AP: Spotpear-Mini", Color565::Orange);
      gfx.drawCenteredText(98, "http://mini.local", Color565::NeonCyan);
    }

    gfx.drawFastHLine(0, 114, SCREEN_WIDTH, Color565::Navy);
    gfx.drawCenteredText(118, "<-/->:VIEW  BOOT:WAKE", Color565::GrayMid);
  }

  void renderWeather(DisplayCanvas& gfx) {
    gfx.fillGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(25, 60, 130), Color565::rgb(10, 20, 40));

    gfx.drawCenteredText(6, gSettings.data.city_name, Color565::White);

    // Sun + Cloud icon
    gfx.fillCircle(34, 44, 13, Color565::Yellow);
    gfx.drawCircle(34, 44, 16, Color565::Gold);
    gfx.fillRoundRect(28, 48, 24, 10, 4, Color565::White);

    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "%.1fC", _tempC);
    gfx.drawText(60, 36, tbuf, Color565::White, 2);

    gfx.fillRoundRect(10, 72, 108, 34, 4, Color565::rgb(15, 30, 60));
    gfx.drawRoundRect(10, 72, 108, 34, 4, Color565::Cyan);

    char hbuf[24];
    snprintf(hbuf, sizeof(hbuf), "Humidity: %u%%", _humidity);
    gfx.drawCenteredText(80, hbuf, Color565::Cyan);
    gfx.drawCenteredText(93, gSettings.data.idle_random_mode ? "Mode: Random Mix" : "Mode: Sequential", Color565::Lime);

    gfx.drawCenteredText(118, "<-/->:VIEW  BOOT:WAKE", Color565::GrayLight);
  }

  void renderPhoto(DisplayCanvas& gfx) {
    if (_photoCount == 0) {
      gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::Black);
      gfx.drawRoundRect(4, 4, 120, 120, 6, Color565::Magenta);
      gfx.drawCenteredText(30, "PHOTO FRAME", Color565::Magenta);
      gfx.drawCenteredText(52, "Upload photos at:", Color565::White);
      gfx.drawCenteredText(68, "http://mini.local", Color565::NeonCyan);
      return;
    }

    if (!_photoLoaded) {
      File f = LittleFS.open(_photoFiles[_curPhotoIdx], "r");
      if (f && f.size() >= SCREEN_WIDTH * SCREEN_HEIGHT * 2) {
        uint16_t* dst = gfx.getBuffer();
        uint8_t lineBuf[SCREEN_WIDTH * 2];
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
          f.read(lineBuf, sizeof(lineBuf));
          for (int x = 0; x < SCREEN_WIDTH; ++x) {
            dst[y * SCREEN_WIDTH + x] = ((uint16_t)lineBuf[x * 2] << 8) | lineBuf[x * 2 + 1];
          }
        }
        _photoLoaded = true;
      }
      if (f) f.close();
    }
  }

  void renderVideo(DisplayCanvas& gfx) {
    if (_videoCount > 0) {
      if (!_videoFile) openCurrentVideo();
      if (_videoFile) {
        uint32_t frameInterval = 1000UL / max((uint8_t)1, _vidFps);
        if (millis() - _lastVidFrameMs >= frameInterval) {
          _lastVidFrameMs = millis();

          uint32_t frameBytes = (_vidFormat == 1) ? (SCREEN_WIDTH * SCREEN_HEIGHT) : (SCREEN_WIDTH * SCREEN_HEIGHT * 2);
          uint32_t offset = 8 + (uint32_t)_vidCurFrame * frameBytes;
          _videoFile.seek(offset, SeekSet);

          uint16_t* dst = gfx.getBuffer();
          if (_vidFormat == 1) {
            uint8_t lineBuf[SCREEN_WIDTH];
            for (int y = 0; y < SCREEN_HEIGHT; ++y) {
              if (_videoFile.read(lineBuf, SCREEN_WIDTH) != SCREEN_WIDTH) break;
              for (int x = 0; x < SCREEN_WIDTH; ++x) {
                dst[y * SCREEN_WIDTH + x] = expandRgb332(lineBuf[x]);
              }
            }
          } else {
            uint8_t lineBuf[SCREEN_WIDTH * 2];
            for (int y = 0; y < SCREEN_HEIGHT; ++y) {
              if (_videoFile.read(lineBuf, sizeof(lineBuf)) != sizeof(lineBuf)) break;
              for (int x = 0; x < SCREEN_WIDTH; ++x) {
                dst[y * SCREEN_WIDTH + x] = ((uint16_t)lineBuf[x * 2] << 8) | lineBuf[x * 2 + 1];
              }
            }
          }

          _vidCurFrame = (_vidCurFrame + 1) % _vidFrameCount;
        }
        return;
      }
    }

    // Fallback procedural animation if all videos are disabled
    static float angle = 0.0f;
    angle += 0.06f;
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::Black);
    for (int r = 10; r < 60; r += 8) {
      int16_t x0 = 64 + (int16_t)(sinf(angle + r * 0.1f) * 12.0f);
      int16_t y0 = 64 + (int16_t)(cosf(angle + r * 0.1f) * 12.0f);
      uint16_t col = (r % 16 == 0) ? Color565::NeonCyan : (r % 24 == 0 ? Color565::Magenta : Color565::Lime);
      gfx.drawCircle(x0, y0, r, col);
    }
    gfx.drawCenteredText(58, "TV MODE", Color565::White);
  }
};
