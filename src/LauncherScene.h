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
#include "PromptPayQrAsset.h"

#define LAUNCH_NONE     -1
#define LAUNCH_IDLE_TV  -2

class LauncherScene {
private:
  int8_t _selectedIndex;
  float  _animOffset;
  bool   _adjustingBri;
  bool   _viewingSupport;
  GameRegistry& _registry;

  static inline uint16_t expandRgb332(uint8_t c) {
    uint8_t r3 = (c >> 5) & 0x07;
    uint8_t g3 = (c >> 2) & 0x07;
    uint8_t b2 = c & 0x03;
    uint16_t r5 = (r3 << 2) | (r3 >> 1);
    uint16_t g6 = (g3 << 3) | g3;
    uint16_t b5 = (b2 << 3) | (b2 << 1) | (b2 >> 1);
    return (r5 << 11) | (g6 << 5) | b5;
  }

  int8_t getActiveGameCount() const {
    int8_t cnt = 0;
    uint32_t mask = gSettings.data.game_enabled_mask & ((1UL << TOTAL_GAMES) - 1);
    if (mask == 0) return 1;
    for (int i = 0; i < TOTAL_GAMES; ++i) {
      uint8_t g = gSettings.data.game_order[i];
      if (g < TOTAL_GAMES && (mask & (1UL << g))) cnt++;
    }
    return (cnt > 0) ? cnt : 1;
  }

  uint8_t getRawGameIndex(int8_t slot) const {
    uint32_t mask = gSettings.data.game_enabled_mask & ((1UL << TOTAL_GAMES) - 1);
    if (mask == 0) return 0;
    int8_t seen = 0;
    for (int i = 0; i < TOTAL_GAMES; ++i) {
      uint8_t g = gSettings.data.game_order[i];
      if (g < TOTAL_GAMES && (mask & (1UL << g))) {
        if (seen == slot) return g;
        seen++;
      }
    }
    return 0;
  }

  int8_t getTotalItems() const {
    return getActiveGameCount() + 2; // Active Games + Brightness Card + About/Support Card
  }

  inline bool isBriCard(int8_t idx) const {
    return (idx == getActiveGameCount());
  }

  inline bool isAboutCard(int8_t idx) const {
    return (idx == getActiveGameCount() + 1);
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
      case 9: { // 9: Flappy Bird
        gfx.fillRect(x + 52, y + 4, 12, 13, Color565::Green);
        gfx.fillRect(x + 52, y + 27, 12, 11, Color565::Green);
        gfx.fillCircle(x + 28, y + 21, 7, Color565::Gold);
        gfx.fillCircle(x + 31, y + 19, 2, Color565::Black);
        gfx.fillTriangle(x + 35, y + 20, x + 41, y + 22, x + 35, y + 24, Color565::Orange);
        break;
      }
      case 10: { // 10: 2048 Puzzle
        gfx.fillRoundRect(x + 14, y + 6, 26, 14, 2, Color565::Orange);
        gfx.fillRoundRect(x + 44, y + 6, 26, 14, 2, Color565::Crimson);
        gfx.fillRoundRect(x + 14, y + 22, 26, 14, 2, Color565::Gold);
        gfx.fillRoundRect(x + 44, y + 22, 26, 14, 2, Color565::Lime);
        gfx.drawText(x + 21, y + 25, "2048", Color565::Black);
        break;
      }
      case 11: { // 11: Space Invaders
        for (int c = 0; c < 3; ++c) {
          gfx.fillRect(x + 18 + c * 20, y + 8, 10, 7, Color565::Magenta);
        }
        gfx.fillRect(x + 37, y + 30, 12, 6, Color565::Lime);
        gfx.fillRect(x + 41, y + 26, 4, 4, Color565::Lime);
        gfx.fillRect(x + 42, y + 18, 2, 6, Color565::Gold);
        break;
      }
      case 12: { // 12: Pac-Mini
        gfx.fillCircle(x + 28, y + 21, 10, Color565::Yellow);
        gfx.fillTriangle(x + 28, y + 21, x + 40, y + 14, x + 40, y + 28, Color565::rgb(12, 18, 32));
        gfx.fillCircle(x + 46, y + 21, 2, Color565::White);
        gfx.fillCircle(x + 56, y + 21, 2, Color565::White);
        gfx.fillCircle(x + 68, y + 21, 6, Color565::Red);
        break;
      }
      case 13: { // 13: Simon Says
        gfx.fillRoundRect(x + 10, y + 10, 20, 22, 3, Color565::Red);
        gfx.fillRoundRect(x + 33, y + 10, 20, 22, 3, Color565::Gold);
        gfx.fillRoundRect(x + 56, y + 10, 20, 22, 3, Color565::NeonCyan);
        break;
      }
      case 14: { // 14: Whack-A-Mole
        gfx.fillRoundRect(x + 21, y + 26, 44, 12, 5, Color565::Orange);
        gfx.fillRoundRect(x + 31, y + 10, 24, 22, 8, Color565::rgb(170, 110, 60));
        gfx.fillCircle(x + 38, y + 18, 2, Color565::Black);
        gfx.fillCircle(x + 48, y + 18, 2, Color565::Black);
        break;
      }
      case 15: { // 15: Tower Stack
        gfx.fillRect(x + 18, y + 28, 50, 7, Color565::NeonCyan);
        gfx.fillRect(x + 24, y + 20, 38, 7, Color565::Lime);
        gfx.fillRect(x + 30, y + 12, 26, 7, Color565::Gold);
        break;
      }
      case 16: { // 16: Frogger
        gfx.fillRect(x + 4, y + 8, 78, 12, Color565::DeepBlue);
        gfx.fillRoundRect(x + 16, y + 11, 26, 6, 2, Color565::Orange);
        gfx.fillRect(x + 4, y + 24, 78, 12, Color565::GrayDark);
        gfx.fillCircle(x + 43, y + 28, 5, Color565::Lime);
        break;
      }
      case 17: { // 17: Minesweeper
        for (int r = 0; r < 2; ++r)
          for (int c = 0; c < 3; ++c)
            gfx.fillRect(x + 18 + c * 17, y + 8 + r * 14, 15, 12, Color565::GrayMid);
        gfx.fillRect(x + 35, y + 8, 15, 12, Color565::Navy);
        gfx.drawText(x + 40, y + 10, "F", Color565::Gold);
        gfx.fillRect(x + 52, y + 22, 15, 12, Color565::Crimson);
        gfx.drawText(x + 57, y + 24, "*", Color565::White);
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

  void drawAboutCardArt(DisplayCanvas& gfx, int16_t x, int16_t y) {
    gfx.fillRoundRect(x, y, 86, 42, 4, Color565::rgb(14, 18, 36));
    gfx.drawRoundRect(x, y, 86, 42, 4, Color565::Magenta);

    // Mini QR emblem box on left
    gfx.fillRoundRect(x + 5, y + 6, 24, 30, 2, Color565::rgb(24, 32, 56));
    gfx.drawRoundRect(x + 5, y + 6, 24, 30, 2, Color565::NeonCyan);
    // Draw cute mini QR grid
    gfx.fillRect(x + 8,  y + 9,  5, 5, Color565::White);
    gfx.fillRect(x + 20, y + 9,  5, 5, Color565::White);
    gfx.fillRect(x + 8,  y + 21, 5, 5, Color565::White);
    gfx.fillRect(x + 19, y + 21, 6, 5, Color565::Gold);
    gfx.fillRect(x + 14, y + 28, 6, 5, Color565::Lime);

    // Text on right side
    gfx.drawText(x + 33, y + 7,  APP_VERSION, Color565::Gold);
    gfx.drawText(x + 33, y + 18, "ESP32-C3", Color565::NeonCyan);
    gfx.drawText(x + 33, y + 29, "SUPPORT", Color565::Lime);
  }

  void renderAboutQrPage(DisplayCanvas& gfx) {
    // 1. Dark background
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 20, 44));

    // 2. Top Header Banner: "SUPPORT LYMINI v0.2.5"
    gfx.fillRect(0, 0, SCREEN_WIDTH, 14, Color565::rgb(0, 62, 126));
    gfx.drawCenteredText(3, "SUPPORT DEV (v" APP_VERSION ")", Color565::Gold);

    // 3. PromptPay QR Code (112x112) centered at (8, 15)
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

public:
  LauncherScene(GameRegistry& reg) : _registry(reg), _selectedIndex(0), _animOffset(0.0f), _adjustingBri(false), _viewingSupport(false) {}

  void init(int8_t initialIndex = 0) {
    _selectedIndex = initialIndex;
    if (_selectedIndex < 0) _selectedIndex = 0;
    if (_selectedIndex >= getTotalItems()) _selectedIndex = getTotalItems() - 1;
    _animOffset = 0.0f;
    _adjustingBri = false;
    _viewingSupport = false;
  }

  int8_t getSelectedIndex() const { return _selectedIndex; }

  int8_t update(const InputState& input, float dt, DisplayCanvas& display) {
    // If inside Support / PromptPay QR Screen:
    if (_viewingSupport) {
      if (input.okShortReleased || input.leftPressed || input.rightPressed || input.menuHold) {
        _viewingSupport = false;
      }
      return LAUNCH_NONE;
    }

    // If inside Brightness Adjustment Mode:
    if (_adjustingBri) {
      if (input.leftPressed) {
        int nb = (int)gSettings.data.brightness - 5;
        if (nb < 5) nb = 5;
        gPower.applyMaxBrightnessNow((uint8_t)nb, &display);
      } else if (input.rightPressed) {
        int nb = (int)gSettings.data.brightness + 5;
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

    // Short Press & Release BOOT -> Launch selected game, open Brightness adjuster, or open Support QR!
    if (input.okShortReleased) {
      if (isBriCard(_selectedIndex)) {
        _adjustingBri = true;
        return LAUNCH_NONE;
      }
      if (isAboutCard(_selectedIndex)) {
        _viewingSupport = true;
        return LAUNCH_NONE;
      }
      return (int8_t)getRawGameIndex(_selectedIndex);
    }

    return LAUNCH_NONE;
  }

  void render(DisplayCanvas& gfx, uint8_t holdPct = 0) {
    if (_viewingSupport) {
      renderAboutQrPage(gfx);
      return;
    }

    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 15, 26));

    drawStatusBar(gfx);

    int8_t total = getTotalItems();
    if (_selectedIndex >= total) _selectedIndex = total - 1;
    if (_selectedIndex < 0) _selectedIndex = 0;

    int16_t cardX = 14 + (int16_t)_animOffset;
    int16_t cardY = 20;
    bool isBri = isBriCard(_selectedIndex);
    bool isAbt = isAboutCard(_selectedIndex);
    uint8_t rawIdx = (isBri || isAbt) ? 0 : getRawGameIndex(_selectedIndex);
    uint16_t borderCol = isBri ? Color565::Gold : (isAbt ? Color565::Magenta : _registry.getGame(rawIdx)->themeColor());

    gfx.fillRoundRect(cardX, cardY, 100, 80, 6, Color565::rgb(25, 30, 50));
    gfx.drawRoundRect(cardX, cardY, 100, 80, 6, borderCol);

    // Side Arrows (< and >)
    gfx.drawText(3,   56, "<", _adjustingBri ? Color565::Gold : Color565::NeonCyan);
    gfx.drawText(120, 56, ">", _adjustingBri ? Color565::Gold : Color565::NeonCyan);

    if (isBri) {
      gfx.drawCenteredText(cardY + 6, "BRIGHTNESS", Color565::Gold);
      drawBrightnessCardArt(gfx, cardX + 7, cardY + 18);
      gfx.drawCenteredText(cardY + 65, _adjustingBri ? "<-/->:SET  OK:SAVE" : "PRESS OK TO ADJUST", Color565::Lime);
    } else if (isAbt) {
      gfx.drawCenteredText(cardY + 6, "ABOUT & SUPPORT", Color565::Magenta);
      drawAboutCardArt(gfx, cardX + 7, cardY + 18);
      gfx.drawCenteredText(cardY + 65, "PRESS OK: DONATE QR", Color565::Gold);
    } else {
      IGame* curGame = _registry.getGame(rawIdx);
      // Game Title at Top of Card
      gfx.drawCenteredText(cardY + 6, curGame->title(), Color565::White);

      // Visual Game Cover Art (Replaces text description!)
      drawGameCoverArt(gfx, rawIdx, cardX + 7, cardY + 18);

      // High Score Badge at Bottom of Card
      uint32_t hs = HighScoreStore::getHighScore(curGame->id());
      char hsBuf[20];
      snprintf(hsBuf, sizeof(hsBuf), "BEST: %lu", (unsigned long)hs);
      gfx.drawCenteredText(cardY + 65, hsBuf, Color565::Gold);
    }

    // Page Indicator Dots (or compact bar when > 14 items)
    if (total <= 14) {
      int16_t dotsStartX = (SCREEN_WIDTH - (total * 7)) / 2;
      for (int8_t i = 0; i < total; ++i) {
        if (i == _selectedIndex) {
          uint16_t dotCol = isBriCard(i) ? Color565::Gold : (isAboutCard(i) ? Color565::Magenta : Color565::NeonCyan);
          gfx.fillCircle(dotsStartX + i * 7 + 3, 106, 2, dotCol);
        } else {
          gfx.drawPixel(dotsStartX + i * 7 + 3, 106, Color565::GrayMid);
        }
      }
    } else {
      char pBuf[12];
      snprintf(pBuf, sizeof(pBuf), "%d/%d", _selectedIndex + 1, total);
      gfx.drawText(6, 102, pBuf, Color565::GrayLight);
      int16_t barX = 44, barW = 76;
      gfx.drawRect(barX, 104, barW, 5, Color565::GrayMid);
      int16_t thumbW = max((int16_t)6, (int16_t)(barW / total));
      int16_t thumbX = barX + ((barW - thumbW) * _selectedIndex) / max(1, total - 1);
      uint16_t thumbCol = isBri ? Color565::Gold : (isAbt ? Color565::Magenta : Color565::NeonCyan);
      gfx.fillRect(thumbX, 105, thumbW, 3, thumbCol);
    }

    // Bottom Action Hint / Hold Progress Bar
    gfx.fillRect(0, 113, SCREEN_WIDTH, 15, Color565::rgb(15, 20, 35));
    gfx.drawFastHLine(0, 113, SCREEN_WIDTH, Color565::DeepBlue);

    if (holdPct > 0) {
      int16_t bw = (holdPct * 124) / 100;
      gfx.fillRect(2, 115, bw, 11, Color565::Magenta);
      gfx.drawCenteredText(117, "HOLD: ENTER TV MODE", Color565::White);
    } else if (_adjustingBri) {
      gfx.drawCenteredText(117, "A:-5% B:+5% OK:OK", Color565::Gold);
    } else if (isAbt) {
      gfx.drawCenteredText(117, "BOOT:VIEW PROMPTPAY", Color565::Gold);
    } else {
      gfx.drawCenteredText(117, "BOOT:OPEN  HOLD:IDLE", Color565::Lime);
    }
  }
};
