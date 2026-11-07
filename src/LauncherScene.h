#pragma once
#include <Arduino.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "GameRegistry.h"
#include "HighScoreStore.h"

class LauncherScene {
private:
  int8_t _selectedIndex;
  float _animOffset;
  GameRegistry& _registry;

public:
  LauncherScene(GameRegistry& reg) : _registry(reg), _selectedIndex(0), _animOffset(0.0f) {}

  void init(int8_t initialIndex = 0) {
    _selectedIndex = initialIndex;
    if (_selectedIndex < 0) _selectedIndex = 0;
    if (_selectedIndex >= _registry.getCount()) _selectedIndex = _registry.getCount() - 1;
    _animOffset = 0.0f;
  }

  int8_t getSelectedIndex() const { return _selectedIndex; }

  // Returns index of game to launch, or -1 if still in menu, or -2 for Smart Idle TV mode
  int8_t update(const InputState& input, float dt) {
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

    if (input.okPressed) {
      return _selectedIndex;
    }

    return -1;
  }

  void render(DisplayCanvas& gfx) {
    // Dark Sci-Fi Background with grid
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 15, 26));

    // Top Title Bar
    gfx.fillRect(0, 0, SCREEN_WIDTH, 16, Color565::rgb(20, 25, 45));
    gfx.drawFastHLine(0, 16, SCREEN_WIDTH, Color565::NeonCyan);
    gfx.drawCenteredText(4, "MINI ARCADE", Color565::NeonCyan);

    IGame* curGame = _registry.getGame(_selectedIndex);

    // Main Card (Center: X = 14, Y = 24, W = 100, H = 76)
    int16_t cardX = 14 + (int16_t)_animOffset;
    int16_t cardY = 24;
    gfx.fillRoundRect(cardX, cardY, 100, 76, 6, Color565::rgb(25, 30, 50));
    gfx.drawRoundRect(cardX, cardY, 100, 76, 6, curGame->themeColor());

    // Game Icon / Badge Area
    gfx.fillCircle(cardX + 50, cardY + 22, 14, curGame->themeColor());
    char numBuf[4];
    snprintf(numBuf, sizeof(numBuf), "%d", _selectedIndex + 1);
    gfx.drawText(cardX + 47, cardY + 18, numBuf, Color565::Black);

    // Game Title & Desc
    gfx.drawCenteredText(cardY + 42, curGame->title(), Color565::White);
    gfx.drawCenteredText(cardY + 54, curGame->desc(), Color565::GrayLight);

    // High Score Badge
    uint32_t hs = HighScoreStore::getHighScore(curGame->id());
    char hsBuf[20];
    snprintf(hsBuf, sizeof(hsBuf), "BEST: %lu", (unsigned long)hs);
    gfx.drawCenteredText(cardY + 64, hsBuf, Color565::Gold);

    // Page Indicator Dots
    int8_t total = _registry.getCount();
    int16_t dotsStartX = (SCREEN_WIDTH - (total * 8)) / 2;
    for (int8_t i = 0; i < total; ++i) {
      if (i == _selectedIndex) {
        gfx.fillCircle(dotsStartX + i * 8 + 3, 106, 2, Color565::NeonCyan);
      } else {
        gfx.drawPixel(dotsStartX + i * 8 + 3, 106, Color565::GrayMid);
      }
    }

    // Bottom Action Hint
    gfx.fillRect(0, 114, SCREEN_WIDTH, 14, Color565::rgb(15, 20, 35));
    gfx.drawFastHLine(0, 114, SCREEN_WIDTH, Color565::DeepBlue);
    gfx.drawCenteredText(118, "<- / ->:PICK  BOOT:PLAY", Color565::Lime);
  }
};
