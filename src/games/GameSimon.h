#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameSimon : public IGame {
private:
  uint8_t _seq[32];
  uint8_t _len;
  uint8_t _step;
  bool _showing;
  float _timer;
  int8_t _litPad;

  void addStep() {
    if (_len < 32) _seq[_len++] = random(0, 3);
    _step = 0;
    _showing = true;
    _timer = 0.0f;
    _litPad = -1;
  }

public:
  const char* id() const override { return "simon"; }
  const char* title() const override { return "Simon Says"; }
  const char* desc() const override { return "Repeat the 3 colors!"; }
  uint16_t themeColor() const override { return Color565::Cyan; }

  void init() override {
    _len = 0;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    addStep();
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    if (_showing) {
      _timer += dt;
      if (_litPad < 0 && _timer >= 0.25f) {
        if (_step >= _len) {
          _showing = false;
          _step = 0;
          _litPad = -1;
          return;
        }
        _litPad = _seq[_step];
        _timer = 0.0f;
      } else if (_litPad >= 0 && _timer >= 0.55f) {
        _litPad = -1;
        _step++;
        _timer = 0.0f;
      }
      return;
    }

    int8_t pressed = -1;
    if (input.leftPressed) pressed = 0;
    else if (input.okShortReleased) pressed = 1;
    else if (input.rightPressed) pressed = 2;

    if (pressed >= 0) {
      _litPad = pressed;
      _timer = 0.28f;
      if (pressed == _seq[_step]) {
        _step++;
        if (_step >= _len) {
          _score = _len * 10;
          addStep();
        }
      } else {
        triggerGameOver();
      }
    } else if (_litPad >= 0) {
      _timer -= dt;
      if (_timer <= 0) _litPad = -1;
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillGradientV(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(10, 14, 28), Color565::rgb(18, 24, 46));
    gfx.drawCenteredText(20, _showing ? "WATCH SEQUENCE" : "YOUR TURN!", _showing ? Color565::Gold : Color565::Lime);

    const uint16_t dim[3] = { Color565::rgb(90, 18, 28), Color565::rgb(95, 76, 12), Color565::rgb(12, 74, 96) };
    const uint16_t bri[3] = { Color565::Red, Color565::Gold, Color565::NeonCyan };
    const char* lbl[3] = { "A", "BOOT", "B" };
    int16_t xs[3] = { 8, 46, 84 };
    for (int i = 0; i < 3; ++i) {
      bool lit = (_litPad == i);
      uint16_t c = lit ? bri[i] : dim[i];
      if (lit) gfx.drawRoundRect(xs[i] - 2, 42, 40, 58, 8, Color565::White);
      gfx.fillRoundRect(xs[i], 44, 36, 54, 6, c);
      gfx.drawRoundRect(xs[i], 44, 36, 54, 6, bri[i]);
      gfx.drawText(xs[i] + (i == 1 ? 6 : 15), 67, lbl[i], lit ? Color565::Black : Color565::White);
    }

    char buf[24];
    if (_showing) snprintf(buf, sizeof(buf), "ROUND: %u", _len);
    else snprintf(buf, sizeof(buf), "STEP: %u / %u", _step, _len);
    gfx.drawCenteredText(110, buf, Color565::White);

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
