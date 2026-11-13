#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <math.h>
#include <LittleFS.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "SystemSettings.h"
#include "PowerManager.h"
#include "PromptPayQrAsset.h"

class IdleScene {
private:
  // Screens (0..7):
  // 0: Cyber Digital Clock
  // 1: Weather & Temp (Multi-location)
  // 2: Photo Frame
  // 3: Video / TV Animation
  // 4: Analog Clock
  // 5: Flip Clock
  // 6: About Me & PromptPay QR (Mandatory, always enabled)
  // 7: Clock + Weather Combo Screen
  uint8_t  _currentScreen;
  uint8_t  _curWeatherCityIdx;
  uint32_t _screenTimer;
  uint32_t _liveHoldUntilMs;
  bool     _liveBufferActive;

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

  bool isScreenEligible(uint8_t scr) const {
    // Screen 6 (PromptPay QR) is mandatory and ALWAYS enabled!
    if (scr == 6) return true;

    uint8_t mask = gSettings.data.idle_enabled_mask | IDLE_MASK_ABOUT_QR;
    if ((mask & (1 << scr)) == 0) return false;

    // Skip Video playback during Eco Mode unless explicitly enabled
    if (scr == 3 && gPower.isEcoMode() && !gSettings.data.eco_video_enabled) {
      return false;
    }

    // If no NTP time synced, do NOT show Clock screens (0: Digital, 4: Analog, 5: Flip, 7: Clock+Weather)
    if ((scr == 0 || scr == 4 || scr == 5 || scr == 7) && !gPower.isTimeSynced()) {
      return false;
    }

    // If no valid Weather data, do NOT show Weather screens (1: Weather, 7: Clock+Weather)
    if ((scr == 1 || scr == 7) && !gPower.hasValidWeather()) {
      return false;
    }

    return true;
  }

  void advanceWeatherCity() {
    if (gSettings.data.weather_count <= 1) {
      _curWeatherCityIdx = 0;
      return;
    }
    uint8_t validList[MAX_WEATHER_CITIES];
    uint8_t validCnt = 0;
    for (uint8_t i = 0; i < gSettings.data.weather_count; ++i) {
      if (gPower.hasValidWeather(i)) {
        validList[validCnt++] = i;
      }
    }
    if (validCnt == 0) {
      _curWeatherCityIdx = 0;
      return;
    }
    if (validCnt == 1) {
      _curWeatherCityIdx = validList[0];
      return;
    }
    // Randomly pick a location (preferring a different city if possible)
    uint8_t pick = validList[random(0, validCnt)];
    if (pick == _curWeatherCityIdx && validCnt > 1) {
      pick = validList[(random(1, validCnt)) % validCnt];
    }
    _curWeatherCityIdx = pick;
  }

  void pickNextScreen(int8_t dir = 1) {
    _liveBufferActive = false;
    _photoLoaded = false;

    uint8_t enabledList[8];
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < 8; ++i) {
      if (isScreenEligible(i)) enabledList[cnt++] = i;
    }

    if (cnt == 0) {
      enabledList[cnt++] = 6; // PromptPay QR is always available
    }

    if (gSettings.data.idle_random_mode && dir == 0 && cnt > 1) {
      _currentScreen = enabledList[random(0, cnt)];
    } else {
      int curPos = -1;
      for (uint8_t i = 0; i < cnt; ++i) {
        if (enabledList[i] == _currentScreen) { curPos = i; break; }
      }
      if (curPos < 0) {
        _currentScreen = enabledList[0];
      } else {
        int step = (dir == 0) ? 1 : dir;
        curPos = (curPos + step + cnt) % cnt;
        _currentScreen = enabledList[curPos];
      }
    }

    if (_currentScreen == 1 || _currentScreen == 7) {
      advanceWeatherCity();
    } else if (_currentScreen == 2 && _photoCount > 0) {
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

  // v0.2.3: Button A (Left) in Idle -> Cycle Clock & Weather Group (0: Digital, 4: Analog, 5: Flip, 1: Weather, 7: Clock+Weather)
  void pickNextClockWeatherScreen() {
    _liveBufferActive = false;
    _photoLoaded = false;

    const uint8_t cwGroup[5] = { 0, 4, 5, 1, 7 };
    uint8_t list[5];
    uint8_t cnt = 0;

    // First try eligible screens in Clock & Weather group
    for (uint8_t i = 0; i < 5; ++i) {
      if (isScreenEligible(cwGroup[i])) list[cnt++] = cwGroup[i];
    }
    // If none are eligible yet (e.g. Wi-Fi/NTP not synced yet), allow cycling through enabled Clock & Weather screens manually
    if (cnt == 0) {
      uint8_t mask = gSettings.data.idle_enabled_mask;
      for (uint8_t i = 0; i < 5; ++i) {
        if (mask & (1 << cwGroup[i])) list[cnt++] = cwGroup[i];
      }
    }
    if (cnt == 0) {
      for (uint8_t i = 0; i < 5; ++i) list[cnt++] = cwGroup[i];
    }

    int curPos = -1;
    for (uint8_t i = 0; i < cnt; ++i) {
      if (list[i] == _currentScreen) { curPos = i; break; }
    }
    if (curPos < 0) {
      _currentScreen = list[0];
    } else {
      _currentScreen = list[(curPos + 1) % cnt];
    }

    if (_currentScreen == 1 || _currentScreen == 7) {
      advanceWeatherCity();
    }
  }

  // v0.2.3: Button B (Right) in Idle -> Cycle Media Gallery Group (2: Photo Frame, 3: Video Animation, 6: PromptPay QR)
  void pickNextMediaGalleryScreen() {
    _liveBufferActive = false;
    _photoLoaded = false;

    const uint8_t mediaGroup[3] = { 2, 3, 6 };
    uint8_t list[3];
    uint8_t cnt = 0;
    uint8_t mask = gSettings.data.idle_enabled_mask | IDLE_MASK_ABOUT_QR;

    for (uint8_t i = 0; i < 3; ++i) {
      if (mask & (1 << mediaGroup[i])) list[cnt++] = mediaGroup[i];
    }
    if (cnt == 0) {
      list[cnt++] = 6;
    }

    int curPos = -1;
    for (uint8_t i = 0; i < cnt; ++i) {
      if (list[i] == _currentScreen) { curPos = i; break; }
    }
    if (curPos < 0) {
      _currentScreen = list[0];
    } else {
      _currentScreen = list[(curPos + 1) % cnt];
    }

    if (_currentScreen == 2 && _photoCount > 0) {
      if (gSettings.data.idle_random_mode && _photoCount > 1) {
        _curPhotoIdx = (_curPhotoIdx + random(1, _photoCount)) % _photoCount;
      } else {
        _curPhotoIdx = (_curPhotoIdx + 1) % _photoCount;
      }
    } else if (_currentScreen == 3 && _videoCount > 0) {
      if (gSettings.data.idle_random_mode && _videoCount > 1) {
        _curVideoIdx = (_curVideoIdx + random(1, _videoCount)) % _videoCount;
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
  IdleScene() : _currentScreen(2), _curWeatherCityIdx(0), _screenTimer(0), _liveHoldUntilMs(0), _liveBufferActive(false),
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
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
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
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
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
    pickNextScreen(0);
  }

  // Triggered from Web Portal "Preview Idle Screen on ESP" button
  void previewScreenFromWeb(uint8_t scrIdx, int8_t cityIdx = -1) {
    scanMediaFiles();
    _screenTimer = millis();
    _liveHoldUntilMs = millis() + 25000UL;
    _liveBufferActive = false;
    _photoLoaded = false;
    _currentScreen = scrIdx % 8;
    if (cityIdx >= 0 && cityIdx < gSettings.data.weather_count) {
      _curWeatherCityIdx = (uint8_t)cityIdx;
    }
    if (_currentScreen == 3 && _videoCount > 0) {
      openCurrentVideo();
    }
  }

  void showPromptPayQrNow() {
    previewScreenFromWeb(6);
  }

  // Triggered from Web Portal Upload or "Show on ESP" button
  void showSpecificMedia(const char* fullPath) {
    if (!fullPath) return;
    scanMediaFiles();
    _screenTimer = millis();
    _liveHoldUntilMs = millis() + 30000UL;
    _liveBufferActive = false; // Crucial: Clear live frame hold so .mvid video plays animated frames!

    String p(fullPath);
    if (p.endsWith(".rgb")) {
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

  void activateLiveViewBuffer() {
    _liveBufferActive = true;
    _liveHoldUntilMs = millis() + 10000UL;
    _screenTimer = millis();
  }

  void clearLiveViewBuffer() {
    _liveBufferActive = false;
  }

  void closeActiveVideo() {
    if (_videoFile) _videoFile.close();
  }

  bool update(const InputState& input, float dt) {
    if (input.okShortReleased || input.menuHold) {
      if (_videoFile) _videoFile.close();
      _liveBufferActive = false;
      return true;
    }

    if (!_liveBufferActive && !isScreenEligible(_currentScreen) && millis() > _liveHoldUntilMs) {
      if (_currentScreen == 3) closeActiveVideo();
      pickNextScreen(0);
    }

    // v0.2.3: Button A (leftPressed) = Clock & Weather group | Button B (rightPressed) = Media Gallery group
    if (input.leftPressed) {
      _liveHoldUntilMs = millis() + 15000UL;
      pickNextClockWeatherScreen();
      _screenTimer = millis();
    } else if (input.rightPressed) {
      _liveHoldUntilMs = millis() + 15000UL;
      pickNextMediaGalleryScreen();
      _screenTimer = millis();
    }

    // Auto-cycle idle screens (unless held by Web Preview / Live View)
    uint32_t cycleMs = (uint32_t)max((uint16_t)5, gSettings.data.idle_cycle_sec) * 1000UL;
    if (millis() > _liveHoldUntilMs && (millis() - _screenTimer >= cycleMs)) {
      _screenTimer = millis();
      pickNextScreen(0);
    }

    return false;
  }

  void render(DisplayCanvas& gfx) {
    if (_liveBufferActive && millis() <= _liveHoldUntilMs) {
      return;
    }
    _liveBufferActive = false;

    switch (_currentScreen) {
      case 0: renderClock(gfx);             break;
      case 1: renderWeather(gfx);           break;
      case 2: renderPhoto(gfx);             break;
      case 3: renderVideo(gfx);             break;
      case 4: renderAnalogClock(gfx);       break;
      case 5: renderFlipClock(gfx);         break;
      case 6: renderAboutQr(gfx);           break;
      case 7: renderClockWeatherCombo(gfx); break;
      default: renderPhoto(gfx);            break;
    }
  }

  // 0: Cyber Digital Clock
  void renderClock(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 12, 24));

    gfx.fillRect(0, 0, SCREEN_WIDTH, 15, Color565::DeepBlue);
    gfx.drawFastHLine(0, 15, SCREEN_WIDTH, Color565::NeonCyan);
    gfx.drawCenteredText(4, "LYMINI CLOCK", Color565::Cyan);

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
      snprintf(timeStr, sizeof(timeStr), "--:--:--");
      snprintf(dateStr, sizeof(dateStr), "Waiting NTP Sync");
    }

    gfx.fillRoundRect(8, 26, 112, 34, 4, Color565::rgb(20, 26, 48));
    gfx.drawRoundRect(8, 26, 112, 34, 4, Color565::NeonCyan);
    gfx.drawCenteredText(36, timeStr, Color565::White, 2);

    gfx.drawCenteredText(68, dateStr, Color565::Gold);

    if (!gPower.isWifiEnabled()) {
      if (gPower.isEcoMode()) {
        gfx.drawCenteredText(86, "ECO MODE (WiFi OFF)", Color565::Lime);
      } else {
        gfx.drawCenteredText(86, "IDLE TV (WiFi OFF)", Color565::Lime);
      }
      char bStr[24];
      snprintf(bStr, sizeof(bStr), "CPU %uMHz | BATT %u%%", gPower.getCpuMhz(), gPower.getBatteryPercent());
      gfx.drawCenteredText(98, bStr, Color565::SkyBlue);
    } else if (WiFi.status() == WL_CONNECTED) {
      gfx.drawCenteredText(86, WiFi.localIP().toString().c_str(), Color565::Lime);
      gfx.drawCenteredText(98, "http://lymini.local", Color565::NeonCyan);
    } else {
      gfx.drawCenteredText(86, "AP: LyMini-AP", Color565::Orange);
      gfx.drawCenteredText(98, "http://lymini.local", Color565::NeonCyan);
    }

    gfx.drawFastHLine(0, 114, SCREEN_WIDTH, Color565::Navy);
    gfx.drawCenteredText(118, "A/B:VIEW  BOOT:WAKE", Color565::GrayMid);
  }

  // 7: Clock + Weather Combo Screen (Shows BOTH Time & Weather together)
  void renderClockWeatherCombo(DisplayCanvas& gfx) {
    gfx.fillGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(12, 22, 48), Color565::rgb(8, 12, 24));

    // Top Header
    gfx.fillRect(0, 0, SCREEN_WIDTH, 14, Color565::rgb(18, 36, 76));
    gfx.drawFastHLine(0, 14, SCREEN_WIDTH, Color565::NeonCyan);
    gfx.drawCenteredText(3, "LYMINI LIVE STATION", Color565::NeonCyan);

    // Upper Card: Digital Clock & Date (Y=18..62)
    gfx.fillRoundRect(6, 18, 116, 44, 5, Color565::rgb(16, 24, 44));
    gfx.drawRoundRect(6, 18, 116, 44, 5, Color565::Cyan);

    char timeStr[16];
    char dateStr[24];
    if (gPower.isTimeSynced()) {
      time_t now = time(nullptr);
      struct tm ti;
      localtime_r(&now, &ti);
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);
      const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
      const char* days[]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
      snprintf(dateStr, sizeof(dateStr), "%s %02d %s %d", days[ti.tm_wday % 7], ti.tm_mday, months[ti.tm_mon % 12], ti.tm_year + 1900);
    } else {
      snprintf(timeStr, sizeof(timeStr), "--:--:--");
      snprintf(dateStr, sizeof(dateStr), "Time Not Synced");
    }
    gfx.drawCenteredText(24, timeStr, Color565::White, 2);
    gfx.drawCenteredText(48, dateStr, Color565::Gold);

    // Lower Card: Multi-Location Weather (Y=66..114)
    gfx.fillRoundRect(6, 66, 116, 46, 5, Color565::rgb(16, 32, 58));
    gfx.drawRoundRect(6, 66, 116, 46, 5, Color565::Gold);

    int cIdx = _curWeatherCityIdx % max((uint8_t)1, gSettings.data.weather_count);
    gfx.drawCenteredText(70, gPower.getWeatherCityName(cIdx), Color565::Yellow);

    // Sun + Cloud mini icon
    gfx.fillCircle(24, 92, 7, Color565::Yellow);
    gfx.fillRoundRect(20, 95, 16, 6, 2, Color565::White);

    if (gPower.hasValidWeather(cIdx)) {
      char tbuf[16];
      snprintf(tbuf, sizeof(tbuf), "%.1fC", gPower.getWeatherTempC(cIdx));
      gfx.drawText(44, 83, tbuf, Color565::White, 2);

      char hbuf[20];
      snprintf(hbuf, sizeof(hbuf), "Humid: %u%%", gPower.getWeatherHumidity(cIdx));
      gfx.drawText(44, 101, hbuf, Color565::NeonCyan);
    } else {
      gfx.drawText(42, 88, "No Weather", Color565::GrayLight);
    }

    gfx.drawCenteredText(117, "A/B:VIEW  BOOT:WAKE", Color565::GrayMid);
  }

  // 4: Analog Clock
  void renderAnalogClock(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(8, 12, 22));

    const int16_t cx = 64;
    const int16_t cy = 56;
    const int16_t r  = 46;

    gfx.fillCircle(cx, cy, r + 3, Color565::rgb(25, 38, 68));
    gfx.fillCircle(cx, cy, r, Color565::rgb(14, 20, 36));
    gfx.drawCircle(cx, cy, r + 3, Color565::NeonCyan);
    gfx.drawCircle(cx, cy, r, Color565::Gold);

    for (int i = 0; i < 12; ++i) {
      float ang = (i * 30.0f - 90.0f) * 0.01745329f;
      int16_t x0 = cx + (int16_t)(cosf(ang) * (r - (i % 3 == 0 ? 7 : 4)));
      int16_t y0 = cy + (int16_t)(sinf(ang) * (r - (i % 3 == 0 ? 7 : 4)));
      int16_t x1 = cx + (int16_t)(cosf(ang) * (r - 2));
      int16_t y1 = cy + (int16_t)(sinf(ang) * (r - 2));
      gfx.drawLine(x0, y0, x1, y1, (i % 3 == 0) ? Color565::Gold : Color565::GrayLight);
    }

    gfx.drawText(cx - 5, cy - r + 9, "12", Color565::NeonCyan);
    gfx.drawText(cx + r - 13, cy - 3, "3", Color565::NeonCyan);
    gfx.drawText(cx - 2, cy + r - 14, "6", Color565::NeonCyan);
    gfx.drawText(cx - r + 9, cy - 3, "9", Color565::NeonCyan);

    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);

    float hAngle = ((ti.tm_hour % 12) * 30.0f + ti.tm_min * 0.5f - 90.0f) * 0.01745329f;
    int16_t hx = cx + (int16_t)(cosf(hAngle) * 23);
    int16_t hy = cy + (int16_t)(sinf(hAngle) * 23);
    gfx.drawLine(cx, cy, hx, hy, Color565::White);
    gfx.drawLine(cx + 1, cy, hx + 1, hy, Color565::White);

    float mAngle = (ti.tm_min * 6.0f + ti.tm_sec * 0.1f - 90.0f) * 0.01745329f;
    int16_t mx = cx + (int16_t)(cosf(mAngle) * 34);
    int16_t my = cy + (int16_t)(sinf(mAngle) * 34);
    gfx.drawLine(cx, cy, mx, my, Color565::NeonCyan);
    gfx.drawLine(cx + 1, cy, mx + 1, my, Color565::NeonCyan);

    float sAngle = (ti.tm_sec * 6.0f - 90.0f) * 0.01745329f;
    int16_t sx = cx + (int16_t)(cosf(sAngle) * 38);
    int16_t sy = cy + (int16_t)(sinf(sAngle) * 38);
    gfx.drawLine(cx, cy, sx, sy, Color565::Red);

    gfx.fillCircle(cx, cy, 3, Color565::Gold);

    const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char footer[24];
    snprintf(footer, sizeof(footer), "%02d:%02d  %d %s", ti.tm_hour, ti.tm_min, ti.tm_mday, months[ti.tm_mon % 12]);
    gfx.fillRoundRect(16, 110, 96, 15, 4, Color565::rgb(18, 28, 48));
    gfx.drawRoundRect(16, 110, 96, 15, 4, Color565::DeepBlue);
    gfx.drawCenteredText(114, footer, Color565::White);
  }

  // 5: Retro Split-Flap Flip Clock
  void renderFlipClock(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(12, 12, 16));

    gfx.drawCenteredText(6, "LYMINI FLIP CLOCK", Color565::Gold);

    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);

    char hStr[4], mStr[4];
    snprintf(hStr, sizeof(hStr), "%02d", ti.tm_hour);
    snprintf(mStr, sizeof(mStr), "%02d", ti.tm_min);

    gfx.fillRoundRect(6, 22, 54, 64, 6, Color565::rgb(30, 32, 40));
    gfx.drawRoundRect(6, 22, 54, 64, 6, Color565::GrayMid);
    gfx.drawText(16, 43, hStr, Color565::White, 3);
    gfx.fillRect(6, 53, 54, 2, Color565::Black);
    gfx.fillRect(7, 51, 3, 6, Color565::GrayLight);
    gfx.fillRect(56, 51, 3, 6, Color565::GrayLight);

    gfx.fillRoundRect(68, 22, 54, 64, 6, Color565::rgb(30, 32, 40));
    gfx.drawRoundRect(68, 22, 54, 64, 6, Color565::GrayMid);
    gfx.drawText(78, 43, mStr, Color565::NeonCyan, 3);
    gfx.fillRect(68, 53, 54, 2, Color565::Black);
    gfx.fillRect(69, 51, 3, 6, Color565::GrayLight);
    gfx.fillRect(118, 51, 3, 6, Color565::GrayLight);

    if ((ti.tm_sec % 2) == 0) {
      gfx.fillCircle(64, 44, 2, Color565::Gold);
      gfx.fillCircle(64, 64, 2, Color565::Gold);
    }

    int16_t secW = (ti.tm_sec * 108) / 59;
    gfx.drawRect(10, 94, 108, 6, Color565::GrayDark);
    gfx.fillRect(10, 94, secW, 6, Color565::Orange);

    const char* days[]   = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    const char* months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
    char dateBuf[24];
    snprintf(dateBuf, sizeof(dateBuf), "%s %02d %s %d",
             days[ti.tm_wday % 7], ti.tm_mday, months[ti.tm_mon % 12], ti.tm_year + 1900);
    gfx.drawCenteredText(108, dateBuf, Color565::GrayLight);
  }

  // 1: Weather & Temp (Multi-location)
  void renderWeather(DisplayCanvas& gfx) {
    gfx.fillGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(25, 60, 130), Color565::rgb(10, 20, 40));

    int cIdx = _curWeatherCityIdx % max((uint8_t)1, gSettings.data.weather_count);
    gfx.drawCenteredText(6, gPower.getWeatherCityName(cIdx), Color565::Yellow);

    gfx.fillCircle(32, 44, 12, Color565::Yellow);
    gfx.drawCircle(32, 44, 15, Color565::Gold);
    gfx.fillRoundRect(26, 48, 24, 10, 4, Color565::White);

    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "%.1fC", gPower.getWeatherTempC(cIdx));
    gfx.drawText(56, 36, tbuf, Color565::White, 2);

    gfx.fillRoundRect(10, 72, 108, 34, 4, Color565::rgb(15, 30, 60));
    gfx.drawRoundRect(10, 72, 108, 34, 4, Color565::Cyan);

    char hbuf[24];
    snprintf(hbuf, sizeof(hbuf), "Humidity: %u%%", gPower.getWeatherHumidity(cIdx));
    gfx.drawCenteredText(80, hbuf, Color565::Cyan);

    char locInfo[24];
    snprintf(locInfo, sizeof(locInfo), "City %d/%d", cIdx + 1, gSettings.data.weather_count);
    gfx.drawCenteredText(93, locInfo, Color565::Lime);

    gfx.drawCenteredText(118, "A/B:VIEW  BOOT:WAKE", Color565::GrayLight);
  }

  // 2: Photo Frame
  void renderPhoto(DisplayCanvas& gfx) {
    if (_photoCount == 0) {
      gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::Black);
      gfx.drawRoundRect(4, 4, 120, 120, 6, Color565::Magenta);
      gfx.drawCenteredText(30, "PHOTO FRAME", Color565::Magenta);
      gfx.drawCenteredText(52, "Upload photos at:", Color565::White);
      gfx.drawCenteredText(68, "http://lymini.local", Color565::NeonCyan);
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

  // 3: Video / Animation
  void renderVideo(DisplayCanvas& gfx) {
    if (_videoCount > 0) {
      if (!_videoFile) openCurrentVideo();
      if (_videoFile) {
        uint32_t frameInterval = 1000UL / max((uint8_t)1, _vidFps);
        if (_lastVidFrameMs == 0 || (millis() - _lastVidFrameMs >= frameInterval)) {
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

          _vidCurFrame = (_vidCurFrame + 1) % max((uint8_t)1, _vidFrameCount);
        }
        return;
      }
    }

    static float angle = 0.0f;
    angle += 0.06f;
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::Black);
    for (int r = 10; r < 60; r += 8) {
      int16_t x0 = 64 + (int16_t)(sinf(angle + r * 0.1f) * 12.0f);
      int16_t y0 = 64 + (int16_t)(cosf(angle + r * 0.1f) * 12.0f);
      uint16_t col = (r % 16 == 0) ? Color565::NeonCyan : (r % 24 == 0 ? Color565::Magenta : Color565::Lime);
      gfx.drawCircle(x0, y0, r, col);
    }
    gfx.drawCenteredText(58, "LYMINI TV", Color565::White);
  }

  // 6: About Me & Buy Me a Coffee (PromptPay QR Code)
  void renderAboutQr(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(12, 24, 52));

    gfx.fillRect(0, 0, SCREEN_WIDTH, 14, Color565::rgb(0, 62, 126));
    gfx.drawCenteredText(3, gSettings.data.idle_lock_qr ? "PROMPTPAY (LOCKED)" : "BUY ME A COFFEE", Color565::Gold);

    const int16_t qx = 8;
    const int16_t qy = 15;
    uint16_t* dst = gfx.getBuffer();
    for (int y = 0; y < PROMPTPAY_QR_SIZE; ++y) {
      int16_t py = qy + y;
      if (py < 0 || py >= SCREEN_HEIGHT) continue;
      for (int x = 0; x < PROMPTPAY_QR_SIZE; ++x) {
        int16_t px = qx + x;
        if (px < 0 || px >= SCREEN_WIDTH) continue;
        uint8_t c332 = pgm_read_byte(&PROMPTPAY_QR_332[y * PROMPTPAY_QR_SIZE + x]);
        dst[py * SCREEN_WIDTH + px] = expandRgb332(c332);
      }
    }
  }
};
