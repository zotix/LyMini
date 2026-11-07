#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <LittleFS.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "SystemSettings.h"

class IdleScene {
private:
  uint8_t _currentScreen; // 0: Clock, 1: Weather, 2: Photo, 3: Video
  uint32_t _screenTimer;

  // Weather state
  float _tempC;
  uint8_t _humidity;
  char _weatherDesc[24];
  uint32_t _lastWeatherFetch;

  // Photo & Video state
  char _photoFiles[8][32];
  uint8_t _photoCount;
  uint8_t _curPhotoIdx;

  File _videoFile;
  bool _videoPlaying;

public:
  IdleScene() : _currentScreen(0), _screenTimer(0), _tempC(28.5f), _humidity(65),
                _lastWeatherFetch(0), _photoCount(0), _curPhotoIdx(0), _videoPlaying(false) {
    strncpy(_weatherDesc, "Clear Sky", sizeof(_weatherDesc));
  }

  void scanMediaFiles() {
    _photoCount = 0;
    if (LittleFS.begin(true)) {
      File dir = LittleFS.open("/photos");
      if (dir && dir.isDirectory()) {
        File file = dir.openNextFile();
        while (file && _photoCount < 8) {
          if (!file.isDirectory() && String(file.name()).endsWith(".rgb")) {
            strncpy(_photoFiles[_photoCount], file.path(), 31);
            _photoCount++;
          }
          file = dir.openNextFile();
        }
      }
    }
  }

  void init() {
    _screenTimer = millis();
    scanMediaFiles();
    fetchWeatherIfNeeded();
  }

  void fetchWeatherIfNeeded() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (_lastWeatherFetch > 0 && millis() - _lastWeatherFetch < 600000) return; // 10 min cache

    _lastWeatherFetch = millis();
    HTTPClient http;
    // Free Open-Meteo API (No API key needed!)
    char url[128];
    snprintf(url, sizeof(url), "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m",
             gSettings.data.latitude, gSettings.data.longitude);

    http.begin(url);
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      int tempIdx = payload.indexOf("\"temperature_2m\":");
      if (tempIdx > 0) {
        _tempC = payload.substring(tempIdx + 17).toFloat();
      }
      int humIdx = payload.indexOf("\"relative_humidity_2m\":");
      if (humIdx > 0) {
        _humidity = payload.substring(humIdx + 23).toInt();
      }
    }
    http.end();
  }

  // Returns true if player pressed button to wake up back to game!
  bool update(const InputState& input, float dt) {
    if (input.okPressed || input.menuHold) {
      if (_videoFile) _videoFile.close();
      return true; // Wake up!
    }

    // Switch Idle Screen manually using Left/Right
    if (input.leftPressed) {
      _currentScreen = (_currentScreen + 3) % 4;
      _screenTimer = millis();
    } else if (input.rightPressed) {
      _currentScreen = (_currentScreen + 1) % 4;
      _screenTimer = millis();
    }

    // Auto-cycle idle screens every 12 seconds
    if (millis() - _screenTimer >= 12000) {
      _screenTimer = millis();
      _currentScreen = (_currentScreen + 1) % 4;
    }

    fetchWeatherIfNeeded();
    return false;
  }

  void render(DisplayCanvas& gfx) {
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
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 10, 20));

    // Top Header
    gfx.fillRect(0, 0, SCREEN_WIDTH, 14, Color565::DeepBlue);
    gfx.drawFastHLine(0, 14, SCREEN_WIDTH, Color565::NeonCyan);
    gfx.drawCenteredText(3, "CYBER CLOCK", Color565::Cyan);

    // Get time from NTP or ESP32 internal RTC
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    char dateStr[24];
    const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    const char* days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    snprintf(dateStr, sizeof(dateStr), "%s, %d %s %d", days[timeinfo.tm_wday], timeinfo.tm_mday, months[timeinfo.tm_mon], timeinfo.tm_year + 1900);

    // Big Digital Clock
    gfx.fillRoundRect(8, 28, 112, 34, 4, Color565::rgb(20, 25, 45));
    gfx.drawRoundRect(8, 28, 112, 34, 4, Color565::NeonCyan);
    gfx.drawCenteredText(38, timeStr, Color565::White, 2);

    // Date
    gfx.drawCenteredText(70, dateStr, Color565::Gold);

    // Status: Wi-Fi & IP
    if (WiFi.status() == WL_CONNECTED) {
      gfx.drawCenteredText(88, "WiFi Connected", Color565::Lime);
      gfx.drawCenteredText(98, WiFi.localIP().toString().c_str(), Color565::SkyBlue);
    } else {
      gfx.drawCenteredText(88, "AP: Spotpear-Mini", Color565::Orange);
      gfx.drawCenteredText(98, "http://mini.local", Color565::NeonCyan);
    }

    // Bottom Wake Hint
    gfx.drawFastHLine(0, 114, SCREEN_WIDTH, Color565::Navy);
    gfx.drawCenteredText(118, "PRESS BOOT TO WAKE", Color565::GrayMid);
  }

  void renderWeather(DisplayCanvas& gfx) {
    // Sky blue to deep blue gradient
    gfx.fillGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(25, 60, 130), Color565::rgb(10, 20, 40));

    gfx.drawCenteredText(8, gSettings.data.city_name, Color565::White);

    // Big Sun Icon
    gfx.fillCircle(36, 48, 14, Color565::Yellow);
    gfx.drawCircle(36, 48, 17, Color565::Gold);

    // Temperature text
    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "%.1f C", _tempC);
    gfx.drawText(62, 36, tbuf, Color565::White, 2);

    // Humidity card
    gfx.fillRoundRect(12, 74, 104, 32, 4, Color565::rgb(15, 30, 60));
    gfx.drawRoundRect(12, 74, 104, 32, 4, Color565::Cyan);

    char hbuf[24];
    snprintf(hbuf, sizeof(hbuf), "Humidity: %u%%", _humidity);
    gfx.drawCenteredText(82, hbuf, Color565::Cyan);
    gfx.drawCenteredText(94, "Forecast: Clear", Color565::Lime);

    gfx.drawCenteredText(118, "PRESS BOOT TO WAKE", Color565::GrayLight);
  }

  void renderPhoto(DisplayCanvas& gfx) {
    if (_photoCount == 0) {
      // Default built-in artwork if no photos uploaded yet
      gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::Black);
      gfx.drawRoundRect(4, 4, 120, 120, 6, Color565::Magenta);
      gfx.drawCenteredText(25, "PHOTO FRAME", Color565::Magenta);
      gfx.drawCenteredText(45, "No .rgb photos", Color565::White);
      gfx.drawCenteredText(60, "Upload via:", Color565::Gold);
      gfx.drawCenteredText(75, "http://mini.local", Color565::NeonCyan);
      gfx.drawCenteredText(100, "PRESS BOOT WAKE", Color565::GrayMid);
      return;
    }

    // Read 128x128 RGB565 raw file from LittleFS directly into display buffer
    File f = LittleFS.open(_photoFiles[_curPhotoIdx], "r");
    if (f) {
      f.read((uint8_t*)gfx.getBuffer(), SCREEN_WIDTH * SCREEN_HEIGHT * 2);
      f.close();
    }
  }

  void renderVideo(DisplayCanvas& gfx) {
    // Built-in Psychedelic / Starfield Animation
    static float angle = 0.0f;
    angle += 0.05f;

    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::Black);

    // Draw hypnotic rotating neon tunnel
    for (int r = 10; r < 60; r += 8) {
      int16_t x0 = 64 + (int16_t)(sinf(angle + r * 0.1f) * 12.0f);
      int16_t y0 = 64 + (int16_t)(cosf(angle + r * 0.1f) * 12.0f);
      uint16_t col = (r % 16 == 0) ? Color565::NeonCyan : (r % 24 == 0 ? Color565::Magenta : Color565::Lime);
      gfx.drawCircle(x0, y0, r, col);
    }

    gfx.drawCenteredText(58, "TV MODE", Color565::White);
    gfx.drawCenteredText(118, "PRESS BOOT TO WAKE", Color565::GrayMid);
  }
};
