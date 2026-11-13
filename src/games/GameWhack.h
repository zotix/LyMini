#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameWhack : public IGame {
private:
  uint8_t _holeType[3]; // 0=empty, 1=mole(+10), 2=gold(+25), 3=bomb(-10)
  float _holeTimer[3];
  float _timeLeft;
  int8_t _hitFlash[3];
  int8_t _lastPts[3];

public:
  const char* id() const override { return "whack"; }
  const char* title() const override { return "Whack-A-Mole"; }
  const char* desc() const override { return "Hit moles, avoid bombs!"; }
  uint16_t themeColor() const override { return Color565::Emerald; }

  void init() override {
    for (int i = 0; i < 3; ++i) {
      _holeType[i] = 0;
      _holeTimer[i] = random(2, 9) * 0.1f;
      _hitFlash[i] = 0;
      _lastPts[i] = 0;
    }
    _timeLeft = 60.0f;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    _timeLeft -= dt;
    if (_timeLeft <= 0) {
      _timeLeft = 0;
      triggerGameOver();
      return;
    }

    bool hit[3] = { input.leftPressed, input.okPressed, input.rightPressed };
    for (int i = 0; i < 3; ++i) {
      if (hit[i] && _holeType[i] != 0) {
        if (_holeType[i] == 1) { _score += 10; _lastPts[i] = 10; }
        else if (_holeType[i] == 2) { _score += 25; _lastPts[i] = 25; }
        else if (_holeType[i] == 3) {
          _score = (_score >= 10) ? (_score - 10) : 0;
          _lastPts[i] = -10;
        }
        _holeType[i] = 0;
        _holeTimer[i] = 0.25f;
        _hitFlash[i] = 10;
      }

      if (_hitFlash[i] > 0) _hitFlash[i]--;

      _holeTimer[i] -= dt;
      if (_holeTimer[i] <= 0) {
        if (_holeType[i] == 0) {
          int r = random(0, 100);
          _holeType[i] = (r < 68) ? 1 : (r < 88 ? 2 : 3);
          _holeTimer[i] = 1.25f + random(0, 40) * 0.01f;
        } else {
          _holeType[i] = 0;
          _holeTimer[i] = 0.20f + random(0, 45) * 0.01f;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Sky + Meadow gradient
    gfx.fillGradientV(0, 13, SCREEN_WIDTH, 32, Color565::rgb(30, 100, 180), Color565::rgb(80, 165, 225));
    gfx.fillCircle(24, 26, 6, Color565::White);
    gfx.fillCircle(31, 24, 7, Color565::White);
    gfx.fillCircle(94, 28, 5, Color565::White);
    gfx.fillGradientV(0, 45, SCREEN_WIDTH, 83, Color565::rgb(28, 92, 34), Color565::rgb(16, 48, 20));

    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "TIME: %ds", (int)ceilf(_timeLeft));
    gfx.drawCenteredText(18, tbuf, _timeLeft < 10.0f ? Color565::Red : Color565::Gold);

    const char* keys[3] = { "A", "BOOT", "B" };
    int16_t xs[3] = { 24, 64, 104 };
    for (int i = 0; i < 3; ++i) {
      gfx.fillRoundRect(xs[i] - 18, 82, 36, 14, 6, Color565::rgb(52, 30, 14));
      gfx.drawRoundRect(xs[i] - 18, 82, 36, 14, 6, Color565::Orange);
      if (_holeType[i] == 1 || _holeType[i] == 2) {
        uint16_t c = (_holeType[i] == 2) ? Color565::Gold : Color565::rgb(175, 115, 62);
        gfx.fillRoundRect(xs[i] - 12, 56, 24, 30, 8, c);
        if (_holeType[i] == 2) gfx.fillRect(xs[i] - 7, 52, 14, 5, Color565::Yellow); // Crown
        gfx.fillCircle(xs[i] - 4, 66, 2, Color565::Black);
        gfx.fillCircle(xs[i] + 4, 66, 2, Color565::Black);
        gfx.fillCircle(xs[i], 71, 2, Color565::Magenta); // Nose
        gfx.fillRect(xs[i] - 2, 75, 4, 3, Color565::White); // Tooth
      } else if (_holeType[i] == 3) {
        gfx.fillCircle(xs[i], 72, 11, Color565::Black);
        gfx.drawCircle(xs[i], 72, 11, Color565::Red);
        gfx.drawLine(xs[i] + 4, 62, xs[i] + 8, 56, Color565::Gold);
        gfx.fillCircle(xs[i] + 9, 55, 2, Color565::Orange);
        gfx.drawText(xs[i] - 3, 68, "!", Color565::Red);
      } else if (_hitFlash[i] > 0) {
        char pbuf[8];
        snprintf(pbuf, sizeof(pbuf), "%+d", _lastPts[i]);
        gfx.drawText(xs[i] - 9, 60, pbuf, _lastPts[i] > 0 ? Color565::Lime : Color565::Red);
      }
      gfx.drawText(xs[i] - (i == 1 ? 12 : 3), 104, keys[i], Color565::White);
    }

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
