#pragma once
#include <Arduino.h>
#include <time.h>
#include <math.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "GameRegistry.h"
#include "HighScoreStore.h"
#include "PowerManager.h"
#include "SystemSettings.h"

#define LAUNCH_NONE     -1
#define LAUNCH_IDLE_TV  -2

class LauncherScene {
private:
  int8_t _selectedIndex;
  float  _animOffset;
  bool   _adjustingBri;
  GameRegistry& _registry;

  int8_t getTotalItems() const {
    return _registry.getCount() + 1; // 9 Games + 1 Brightness Settings Card = 10 items
  }

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
      gfx.drawText(3, 4, "LyMini", Color565::NeonCyan);
    }

    // 2. Wi-Fi Status (Center: X = 42)
    WifiPowerState ws = gPower.getWifiState();
    int16_t wx = 42;
    if (ws == WIFI_PWR_STA_CONNECTED) {
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

    uint16_t bCol = usb ? Color565::NeonCyan : ((pct > 50) ? Color565::Lime : (pct > 20 ? Color565::Gold : Color565::Red));
    gfx.drawRect(112, 4, 12, 8, Color565::White);
    gfx.fillRect(124, 6, 2, 4, Color565::White);
    int16_t fillW = (pct * 8) / 100;
    if (fillW < 1) fillW = 1;
    if (fillW > 8) fillW = 8;
    gfx.fillRect(114, 6, fillW, 4, bCol);
  }

  // Draw custom visual Game Cover Art inside box (x, y, w=86, h=42)
  void drawGameCoverArt(DisplayCanvas& gfx, uint8_t idx, int16_t x, int16_t y) {
    gfx.fillRoundRect(x, y, 86, 42, 4, Color565::rgb(12, 16, 28));
    gfx.drawRoundRect(x, y, 86, 42, 4, Color565::rgb(45, 58, 90));

    switch (idx) {
      case 0: { // 0: Chrome Dino
        gfx.drawFastHLine(x + 4, y + 34, 78, Color565::Gold);
        // Sun
        gfx.fillCircle(x + 70, y + 11, 5, Color565::Yellow);
        // Dino jumping
        gfx.fillRect(x + 18, y + 12, 10, 8, Color565::Lime);
        gfx.fillRect(x + 14, y + 18, 10, 9, Color565::Lime);
        gfx.drawPixel(x + 25, y + 14, Color565::Black);
        gfx.fillRect(x + 15, y + 27, 3, 4, Color565::Lime);
        gfx.fillRect(x + 21, y + 27, 3, 4, Color565::Lime);
        // Cacti
        gfx.fillRect(x + 50, y + 22, 4, 12, Color565::Emerald);
        gfx.fillRect(x + 47, y + 25, 3, 5, Color565::Emerald);
        gfx.fillRect(x + 54, y + 24, 3, 5, Color565::Emerald);
        break;
      }
      case 1: { // 1: Cyber Pong
        for (int py = y + 4; py < y + 38; py += 6) {
          gfx.drawFastVLine(x + 43, py, 3, Color565::GrayMid);
        }
        gfx.fillRect(x + 8,  y + 10, 4, 16, Color565::NeonCyan);
        gfx.fillRect(x + 74, y + 16, 4, 16, Color565::Magenta);
        gfx.fillCircle(x + 52, y + 20, 3, Color565::Yellow);
        gfx.drawLine(x + 36, y + 14, x + 49, y + 19, Color565::Orange);
        break;
      }
      case 2: { // 2: Space Runner
        gfx.drawPixel(x + 12, y + 8, Color565::White);
        gfx.drawPixel(x + 68, y + 10, Color565::White);
        gfx.drawPixel(x + 45, y + 34, Color565::White);
        // Spaceship
        gfx.fillTriangle(x + 14, y + 14, x + 14, y + 28, x + 30, y + 21, Color565::NeonCyan);
        gfx.fillTriangle(x + 10, y + 18, x + 10, y + 24, x + 14, y + 21, Color565::Orange);
        // Laser beam
        gfx.fillRect(x + 34, y + 20, 14, 2, Color565::Lime);
        // Asteroid
        gfx.fillCircle(x + 64, y + 21, 9, Color565::Coral);
        gfx.fillCircle(x + 61, y + 18, 2, Color565::Red);
        break;
      }
      case 3: { // 3: Car Dodge
        gfx.fillRect(x + 18, y + 2, 50, 38, Color565::rgb(35, 38, 46));
        gfx.drawFastVLine(x + 18, y + 2, 38, Color565::White);
        gfx.drawFastVLine(x + 68, y + 2, 38, Color565::White);
        for (int py = y + 4; py < y + 38; py += 8) {
          gfx.drawFastVLine(x + 43, py, 4, Color565::Yellow);
        }
        // Player Red Racecar
        gfx.fillRoundRect(x + 25, y + 22, 12, 15, 2, Color565::Red);
        gfx.fillRect(x + 27, y + 25, 8, 4, Color565::NeonCyan);
        // Rival Car
        gfx.fillRoundRect(x + 49, y + 6, 12, 15, 2, Color565::Gold);
        gfx.fillRect(x + 51, y + 14, 8, 4, Color565::SkyBlue);
        break;
      }
      case 4: { // 4: Retro Snake
        gfx.drawRect(x + 6, y + 5, 74, 32, Color565::Gold);
        // Snake segments
        gfx.fillRect(x + 16, y + 24, 6, 6, Color565::Mint);
        gfx.fillRect(x + 23, y + 24, 6, 6, Color565::Emerald);
        gfx.fillRect(x + 30, y + 24, 6, 6, Color565::Mint);
        gfx.fillRect(x + 30, y + 17, 6, 6, Color565::Emerald);
        gfx.fillRect(x + 37, y + 17, 6, 6, Color565::Mint);
        gfx.fillRoundRect(x + 44, y + 16, 7, 7, 2, Color565::Lime); // Head
        gfx.drawPixel(x + 49, y + 18, Color565::Black);
        // Apple
        gfx.fillCircle(x + 64, y + 19, 4, Color565::Red);
        gfx.drawPixel(x + 64, y + 14, Color565::Lime);
        break;
      }
      case 5: { // 5: Timberman
        // Tree trunk
        gfx.fillRect(x + 35, y + 2, 16, 38, Color565::rgb(140, 80, 35));
        gfx.drawFastVLine(x + 40, y + 2, 38, Color565::rgb(100, 55, 20));
        // Branches
        gfx.fillRect(x + 15, y + 8, 20, 5, Color565::Emerald);
        gfx.fillRect(x + 51, y + 18, 20, 5, Color565::Emerald);
        // Lumberjack on left
        gfx.fillCircle(x + 22, y + 25, 4, Color565::Gold);
        gfx.fillRect(x + 19, y + 29, 7, 9, Color565::Red);
        gfx.drawLine(x + 26, y + 30, x + 34, y + 26, Color565::White); // Axe
        break;
      }
      case 6: { // 6: Cyber Tetris
        // T-piece (Purple)
        gfx.fillRect(x + 16, y + 26, 21, 7, Color565::Purple);
        gfx.fillRect(x + 23, y + 19, 7,  7, Color565::Purple);
        // L-piece (Orange)
        gfx.fillRect(x + 40, y + 26, 21, 7, Color565::Orange);
        gfx.fillRect(x + 54, y + 19, 7,  7, Color565::Orange);
        // Falling Cyan Bar
        gfx.fillRect(x + 28, y + 7,  28, 6, Color565::NeonCyan);
        break;
      }
      case 7: { // 7: Neon Breakout
        uint16_t cols[3] = { Color565::Red, Color565::Gold, Color565::Lime };
        for (int r = 0; r < 3; ++r) {
          for (int c = 0; c < 5; ++c) {
            gfx.fillRect(x + 8 + c * 14, y + 5 + r * 5, 12, 4, cols[r]);
          }
        }
        gfx.fillCircle(x + 46, y + 25, 3, Color565::White);
        gfx.fillRoundRect(x + 28, y + 34, 30, 4, 2, Color565::NeonCyan);
        break;
      }
      case 8: { // 8: Cyber Bomber
        // Brick blocks
        gfx.fillRect(x + 10, y + 8,  12, 12, Color565::rgb(180, 75, 30));
        gfx.fillRect(x + 64, y + 22, 12, 12, Color565::rgb(70, 82, 105));
        // Big Bomb in center
        gfx.fillCircle(x + 43, y + 23, 10, Color565::Black);
        gfx.drawCircle(x + 43, y + 23, 10, Color565::White);
        gfx.fillCircle(x + 40, y + 20, 2, Color565::White);
        // Burning fuse spark
        gfx.drawLine(x + 48, y + 14, x + 53, y + 9, Color565::Gold);
        gfx.fillCircle(x + 54, y + 8, 3, Color565::Orange);
        gfx.drawPixel(x + 54, y + 8, Color565::Yellow);
        break;
      }
    }
  }

  void drawBrightnessCardArt(DisplayCanvas& gfx, int16_t x, int16_t y) {
    gfx.fillRoundRect(x, y, 86, 42, 4, Color565::rgb(12, 18, 32));
    gfx.drawRoundRect(x, y, 86, 42, 4, _adjustingBri ? Color565::Gold : Color565::NeonCyan);

    // Sun icon on left (cx = x+22, cy = y+21)
    int16_t sx = x + 20, sy = y + 21;
    gfx.fillCircle(sx, sy, 7, Color565::Yellow);
    gfx.drawCircle(sx, sy, 10, Color565::Gold);

    // Brightness Percentage Text & Bar on right
    char bTxt[12];
    snprintf(bTxt, sizeof(bTxt), "MAX %u%%", gSettings.data.brightness);
    gfx.drawText(x + 35, y + 9, bTxt, Color565::White);

    gfx.drawRect(x + 35, y + 22, 44, 10, Color565::White);
    int16_t fw = (gSettings.data.brightness * 40) / 100;
    if (fw < 2) fw = 2;
    gfx.fillRect(x + 37, y + 24, fw, 6, Color565::Lime);
  }

public:
  LauncherScene(GameRegistry& reg) : _registry(reg), _selectedIndex(0), _animOffset(0.0f), _adjustingBri(false) {}

  void init(int8_t initialIndex = 0) {
    _selectedIndex = initialIndex;
    if (_selectedIndex < 0) _selectedIndex = 0;
    if (_selectedIndex >= getTotalItems()) _selectedIndex = getTotalItems() - 1;
    _animOffset = 0.0f;
    _adjustingBri = false;
  }

  int8_t getSelectedIndex() const { return _selectedIndex; }

  int8_t update(const InputState& input, float dt, DisplayCanvas& display) {
    // If inside Brightness Adjustment Mode:
    if (_adjustingBri) {
      if (input.leftPressed) {
        int nb = (int)gSettings.data.brightness - 10;
        if (nb < 5) nb = 5;
        gPower.applyMaxBrightnessNow((uint8_t)nb, &display);
      } else if (input.rightPressed) {
        int nb = (int)gSettings.data.brightness + 10;
        if (nb > 100) nb = 100;
        gPower.applyMaxBrightnessNow((uint8_t)nb, &display);
      } else if (input.okShortReleased || input.menuHold) {
        gSettings.save();
        _adjustingBri = false;
      }
      return LAUNCH_NONE;
    }

    // Press Left + Right together in menu to toggle Wi-Fi On/Off
    if (input.bothPressed) {
      gPower.toggleWifi();
      return LAUNCH_NONE;
    }

    int8_t total = getTotalItems();
    if (input.leftPressed) {
      _selectedIndex--;
      if (_selectedIndex < 0) _selectedIndex = total - 1;
      _animOffset = -20.0f;
    } else if (input.rightPressed) {
      _selectedIndex++;
      if (_selectedIndex >= total) _selectedIndex = 0;
      _animOffset = 20.0f;
    }

    _animOffset += (0.0f - _animOffset) * 16.0f * dt;

    // Hold BOOT (1.5s) -> Enter Idle TV Mode!
    if (input.menuHold) {
      return LAUNCH_IDLE_TV;
    }

    // Short Press & Release BOOT -> Launch selected game or open Brightness adjuster!
    if (input.okShortReleased) {
      if (_selectedIndex == _registry.getCount()) {
        _adjustingBri = true;
        return LAUNCH_NONE;
      }
      return _selectedIndex;
    }

    return LAUNCH_NONE;
  }

  void render(DisplayCanvas& gfx, uint8_t holdPct = 0) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 15, 26));

    drawStatusBar(gfx);

    int16_t cardX = 14 + (int16_t)_animOffset;
    int16_t cardY = 20;
    bool isBriCard = (_selectedIndex == _registry.getCount());
    uint16_t borderCol = isBriCard ? Color565::Gold : _registry.getGame(_selectedIndex)->themeColor();

    gfx.fillRoundRect(cardX, cardY, 100, 80, 6, Color565::rgb(25, 30, 50));
    gfx.drawRoundRect(cardX, cardY, 100, 80, 6, borderCol);

    // Side Arrows (< and >)
    gfx.drawText(3,   56, "<", _adjustingBri ? Color565::Gold : Color565::NeonCyan);
    gfx.drawText(120, 56, ">", _adjustingBri ? Color565::Gold : Color565::NeonCyan);

    if (isBriCard) {
      gfx.drawCenteredText(cardY + 6, "BRIGHTNESS", Color565::Gold);
      drawBrightnessCardArt(gfx, cardX + 7, cardY + 18);
      gfx.drawCenteredText(cardY + 65, _adjustingBri ? "<-/->:SET  OK:SAVE" : "PRESS OK TO ADJUST", Color565::Lime);
    } else {
      IGame* curGame = _registry.getGame(_selectedIndex);
      // Game Title at Top of Card
      gfx.drawCenteredText(cardY + 6, curGame->title(), Color565::White);

      // Visual Game Cover Art (Replaces text description!)
      drawGameCoverArt(gfx, _selectedIndex, cardX + 7, cardY + 18);

      // High Score Badge at Bottom of Card
      uint32_t hs = HighScoreStore::getHighScore(curGame->id());
      char hsBuf[20];
      snprintf(hsBuf, sizeof(hsBuf), "BEST: %lu", (unsigned long)hs);
      gfx.drawCenteredText(cardY + 65, hsBuf, Color565::Gold);
    }

    // Page Indicator Dots (10 items)
    int8_t total = getTotalItems();
    int16_t dotsStartX = (SCREEN_WIDTH - (total * 7)) / 2;
    for (int8_t i = 0; i < total; ++i) {
      if (i == _selectedIndex) {
        gfx.fillCircle(dotsStartX + i * 7 + 3, 106, 2, isBriCard ? Color565::Gold : Color565::NeonCyan);
      } else {
        gfx.drawPixel(dotsStartX + i * 7 + 3, 106, Color565::GrayMid);
      }
    }

    // Bottom Action Hint / Hold Progress Bar
    gfx.fillRect(0, 113, SCREEN_WIDTH, 15, Color565::rgb(15, 20, 35));
    gfx.drawFastHLine(0, 113, SCREEN_WIDTH, Color565::DeepBlue);

    if (holdPct > 0) {
      int16_t bw = (holdPct * 124) / 100;
      gfx.fillRect(2, 115, bw, 11, Color565::Magenta);
      gfx.drawCenteredText(117, "HOLD: ENTER TV MODE", Color565::White);
    } else if (_adjustingBri) {
      gfx.drawCenteredText(117, "A:-10% B:+10% OK:OK", Color565::Gold);
    } else {
      gfx.drawCenteredText(117, "BOOT:OPEN  HOLD:IDLE", Color565::Lime);
    }
  }
};
