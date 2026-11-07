#pragma once
#include <Arduino.h>
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "HighScoreStore.h"

class IGame {
protected:
  bool _isGameOver = false;
  uint32_t _score = 0;
  uint32_t _highScore = 0;
  bool _newHighScore = false;

public:
  virtual ~IGame() {}

  virtual const char* id() const = 0;
  virtual const char* title() const = 0;
  virtual const char* desc() const = 0;
  virtual uint16_t themeColor() const = 0;

  virtual void init() = 0;
  virtual void update(const InputState& input, float dt) = 0;
  virtual void render(DisplayCanvas& gfx) = 0;

  virtual bool isGameOver() const { return _isGameOver; }
  virtual uint32_t getScore() const { return _score; }

  virtual void triggerGameOver() {
    _isGameOver = true;
    _newHighScore = HighScoreStore::submitScore(id(), _score);
    _highScore = HighScoreStore::getHighScore(id());
  }

  // Draw Standard Retro Arcade HUD Bar
  virtual void drawHud(DisplayCanvas& gfx) {
    gfx.fillRect(0, 0, SCREEN_WIDTH, 12, Color565::Black);
    gfx.drawFastHLine(0, 12, SCREEN_WIDTH, themeColor());
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)_score);
    gfx.drawText(2, 2, buf, Color565::White);

    snprintf(buf, sizeof(buf), "HI %lu", (unsigned long)_highScore);
    int len = strlen(buf);
    gfx.drawText(SCREEN_WIDTH - (len * 6) - 2, 2, buf, Color565::Gold);
  }

  // Draw Standard Game Over Screen Overlay
  virtual void drawGameOverOverlay(DisplayCanvas& gfx) {
    gfx.fillRoundRect(14, 28, 100, 72, 4, Color565::Black);
    gfx.drawRoundRect(14, 28, 100, 72, 4, _newHighScore ? Color565::Gold : Color565::Red);

    gfx.drawCenteredText(34, "GAME OVER", Color565::Red);

    char buf[20];
    snprintf(buf, sizeof(buf), "SCORE: %lu", (unsigned long)_score);
    gfx.drawCenteredText(48, buf, Color565::White);

    if (_newHighScore) {
      gfx.drawCenteredText(60, "* NEW HIGH! *", Color565::Gold);
    } else {
      snprintf(buf, sizeof(buf), "BEST: %lu", (unsigned long)_highScore);
      gfx.drawCenteredText(60, buf, Color565::Cyan);
    }

    gfx.drawCenteredText(76, "PRESS BOOT/A", Color565::Lime);
    gfx.drawCenteredText(86, "HOLD=MENU", Color565::GrayMid);
  }
};
