#pragma once
#include <Arduino.h>
#include <time.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "GameRegistry.h"
#include "HighScoreStore.h"
#include "PowerManager.h"

#define LAUNCH_NONE     -1
#define LAUNCH_IDLE_TV  -2

class LauncherScene {
private:
  int8_t _selectedIndex;
  float _animOffset;
  GameRegistry& _registry;

  void drawStatusBar(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, 16, Color565::rgb(16, 22, 40));
    gfx.drawFastHLine(0, 16, SCREEN_WIDTH, Color565::NeonCyan);

    // 1. Time (Left: X = 3, Y = 4)
    char timeBuf[8];
    if (gPower.isTimeSynced()) {
      time_t now = time(nullptr);
      struct tm ti;
      localtime_r(&now, &ti);
      bool colonOn = ((millis() / 500) % 2 == 0);
      snprintf(timeBuf, sizeof(timeBuf), colonOn ? "%02d:%02d" : "%02d %02d", ti.tm_hour, ti.tm_min);
      gfx.drawText(3, 4, timeBuf, Color565::White);
    } else {
      gfx.drawText(3, 4, "--:--", Color565::GrayMid);
    }

    // 2. Wi-Fi Status (Center: X = 40)
    WifiPowerState ws = gPower.getWifiState();
    int16_t wx = 40;
    if (ws == WIFI_PWR_STA_CONNECTED) {
      // Draw 3 signal bars
      gfx.drawFastVLine(wx,     9, 3, Color565::Lime);
      gfx.drawFastVLine(wx + 2, 7, 5, Color565::Lime);
      gfx.drawFastVLine(wx + 4, 4, 8, Color565::Lime);
      gfx.drawText(wx + 7, 4, "WiFi", Color565::Lime);
    } else if (ws == WIFI_PWR_AP_ACTIVE) {
      gfx.drawFastVLine(wx,     9, 3, Color565::NeonCyan);
      gfx.drawFastVLine(wx + 2, 7, 5, Color565::NeonCyan);
      gfx.drawFastVLine(wx + 4, 4, 8, Color565::NeonCyan);
      gfx.drawText(wx + 7, 4, "AP+", Color565::NeonCyan);
    } else if (ws == WIFI_PWR_SEARCHING) {
      uint16_t col = ((millis() / 300) % 2 == 0) ? Color565::Orange : Color565::Yellow;
      gfx.drawFastVLine(wx,     9, 3, col);
      gfx.drawFastVLine(wx + 2, 7, 5, col);
      gfx.drawText(wx + 6, 4, "AP..", col);
    } else {
      // Wi-Fi Off (Power Save)
      gfx.drawText(wx + 2, 4, "ECO", Color565::GrayMid);
    }

    // 3. Battery Status (Right: X = 86..126)
    uint8_t pct = gPower.getBatteryPercent();
    bool usb = gPower.isUsbOrCharging();

    char bStr[6];
    if (usb) {
      snprintf(bStr, sizeof(bStr), "USB");
    } else {
      snprintf(bStr, sizeof(bStr), "%u%%", pct);
    }
    int bLen = strlen(bStr);
    int16_t tx = 110 - (bLen * 6);
    gfx.drawText(tx, 4, bStr, usb ? Color565::NeonCyan : Color565::White);

    // Battery Icon at X=112, Y=4 (12x8 px + 2px tip)
    uint16_t bCol = usb ? Color565::NeonCyan : ((pct > 50) ? Color565::Lime : (pct > 20 ? Color565::Gold : Color565::Red));
    gfx.drawRect(112, 4, 12, 8, Color565::White);
    gfx.fillRect(124, 6, 2, 4, Color565::White); // Positive terminal nub
    int16_t fillW = (pct * 8) / 100;
    if (fillW < 1) fillW = 1;
    if (fillW > 8) fillW = 8;
    gfx.fillRect(114, 6, fillW, 4, bCol);
  }

public:
  LauncherScene(GameRegistry& reg) : _registry(reg), _selectedIndex(0), _animOffset(0.0f) {}

  void init(int8_t initialIndex = 0) {
    _selectedIndex = initialIndex;
    if (_selectedIndex < 0) _selectedIndex = 0;
    if (_selectedIndex >= _registry.getCount()) _selectedIndex = _registry.getCount() - 1;
    _animOffset = 0.0f;
  }

  int8_t getSelectedIndex() const { return _selectedIndex; }

  // Returns:
  //   >= 0 : Game index to launch
  //   LAUNCH_NONE (-1) : Stay in menu
  //   LAUNCH_IDLE_TV (-2) : Enter Idle TV Mode immediately (when holding BOOT)
  int8_t update(const InputState& input, float dt) {
    // Press Left + Right together in menu to toggle Wi-Fi On/Off
    if (input.bothPressed) {
      gPower.toggleWifi();
      return LAUNCH_NONE;
    }

    if (input.leftPressed) {
      _selectedIndex--;
      if (_selectedIndex < 0) _selectedIndex = _registry.getCount() - 1;
      _animOffset = -20.0f;
    } else if (input.rightPressed) {
      _selectedIndex++;
      if (_selectedIndex >= _registry.getCount()) _selectedIndex = 0;
      _animOffset = 20.0f;
    }

    // Smooth return to center
    _animOffset += (0.0f - _animOffset) * 16.0f * dt;

    // Hold BOOT (1.2s) -> Enter Idle TV Mode!
    if (input.menuHold) {
      return LAUNCH_IDLE_TV;
    }

    // Short Press & Release BOOT -> Launch selected game!
    if (input.okShortReleased) {
      return _selectedIndex;
    }

    return LAUNCH_NONE;
  }

  void render(DisplayCanvas& gfx, uint8_t holdPct = 0) {
    // Dark Sci-Fi Background
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 15, 26));

    // Top Status Bar (Time | Wi-Fi | Battery)
    drawStatusBar(gfx);

    IGame* curGame = _registry.getGame(_selectedIndex);

    // Main Card (Center: X = 14, Y = 22, W = 100, H = 76)
    int16_t cardX = 14 + (int16_t)_animOffset;
    int16_t cardY = 22;
    gfx.fillRoundRect(cardX, cardY, 100, 76, 6, Color565::rgb(25, 30, 50));
    gfx.drawRoundRect(cardX, cardY, 100, 76, 6, curGame->themeColor());

    // Side Arrows (< and >)
    gfx.drawText(3,  56, "<", Color565::NeonCyan);
    gfx.drawText(120,56, ">", Color565::NeonCyan);

    // Game Icon / Badge Area
    gfx.fillCircle(cardX + 50, cardY + 20, 13, curGame->themeColor());
    char numBuf[4];
    snprintf(numBuf, sizeof(numBuf), "%d", _selectedIndex + 1);
    gfx.drawText(cardX + 47, cardY + 16, numBuf, Color565::Black);

    // Game Title & Desc
    gfx.drawCenteredText(cardY + 39, curGame->title(), Color565::White);
    gfx.drawCenteredText(cardY + 51, curGame->desc(), Color565::GrayLight);

    // High Score Badge
    uint32_t hs = HighScoreStore::getHighScore(curGame->id());
    char hsBuf[20];
    snprintf(hsBuf, sizeof(hsBuf), "BEST: %lu", (unsigned long)hs);
    gfx.drawCenteredText(cardY + 63, hsBuf, Color565::Gold);

    // Page Indicator Dots
    int8_t total = _registry.getCount();
    int16_t dotsStartX = (SCREEN_WIDTH - (total * 8)) / 2;
    for (int8_t i = 0; i < total; ++i) {
      if (i == _selectedIndex) {
        gfx.fillCircle(dotsStartX + i * 8 + 3, 105, 2, Color565::NeonCyan);
      } else {
        gfx.drawPixel(dotsStartX + i * 8 + 3, 105, Color565::GrayMid);
      }
    }

    // Bottom Action Hint / Hold Progress Bar
    gfx.fillRect(0, 113, SCREEN_WIDTH, 15, Color565::rgb(15, 20, 35));
    gfx.drawFastHLine(0, 113, SCREEN_WIDTH, Color565::DeepBlue);

    if (holdPct > 0) {
      int16_t bw = (holdPct * 124) / 100;
      gfx.fillRect(2, 115, bw, 11, Color565::Magenta);
      gfx.drawCenteredText(117, "HOLD: ENTER TV MODE", Color565::White);
    } else {
      gfx.drawCenteredText(117, "BOOT:PLAY  HOLD:IDLE", Color565::Lime);
    }
  }
};
