#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameWhack : public IGame {
private:
  uint8_t _holeType[3]; // 0=empty, 1=mole(+10), 2=gold(+25), 3=bomb(-15/game over? -15)
  float _holeTimer[3];
  float _timeLeft;
  int8_t _hitFlash[3];

public:
  const char* id() const override { return "whack"; }
  const char* title() const override { return "Whack-A-Mole"; }
  const char* desc() const override { return "Hit moles, avoid bombs!"; }
  uint16_t themeColor() const override { return Color565::Emerald; }

  void init() override {
    for (int i = 0; i < 3; ++i) {
      _holeType[i] = 0;
      _holeTimer[i] = random(3, 12) * 0.1f;
      _hitFlash[i] = 0;
    }
    _timeLeft = 45.0f;
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
        if (_holeType[i] == 1) _score += 10;
        else if (_holeType[i] == 2) _score += 25;
        else if (_holeType[i] == 3) {
          _score = (_score >= 15) ? (_score - 15) : 0;
        }
        _holeType[i] = 0;
        _holeTimer[i] = 0.35f;
        _hitFlash[i] = 6;
      }

      if (_hitFlash[i] > 0) _hitFlash[i]--;

      _holeTimer[i] -= dt;
      if (_holeTimer[i] <= 0) {
        if (_holeType[i] == 0) {
          int r = random(0, 100);
          _holeType[i] = (r < 62) ? 1 : (r < 80 ? 2 : 3);
          _holeTimer[i] = 0.75f + random(0, 50) * 0.01f;
        } else {
          _holeType[i] = 0;
          _holeTimer[i] = 0.25f + random(0, 60) * 0.01f;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(20, 48, 22));
    char tbuf[16];
    snprintf(tbuf, sizeof(tbuf), "TIME: %ds", (int)ceilf(_timeLeft));
    gfx.drawCenteredText(18, tbuf, _timeLeft < 10.0f ? Color565::Red : Color565::Gold);

    const char* keys[3] = { "A", "BOOT", "B" };
    int16_t xs[3] = { 24, 64, 104 };
    for (int i = 0; i < 3; ++i) {
      gfx.fillRoundRect(xs[i] - 17, 79, 34, 14, 6, Color565::rgb(40, 24, 12));
      gfx.drawRoundRect(xs[i] - 17, 79, 34, 14, 6, Color565::Orange);
      if (_holeType[i] == 1 || _holeType[i] == 2) {
        uint16_t c = (_holeType[i] == 2) ? Color565::Gold : Color565::rgb(170, 110, 60);
        gfx.fillRoundRect(xs[i] - 11, 56, 22, 28, 8, c);
        gfx.fillCircle(xs[i] - 4, 65, 2, Color565::Black);
        gfx.fillCircle(xs[i] + 4, 65, 2, Color565::Black);
        gfx.fillCircle(xs[i], 70, 2, Color565::Red);
      } else if (_holeType[i] == 3) {
        gfx.fillCircle(xs[i], 70, 11, Color565::Black);
        gfx.drawCircle(xs[i], 70, 11, Color565::Red);
        gfx.drawText(xs[i] - 3, 66, "X", Color565::Red);
      } else if (_hitFlash[i] > 0) {
        gfx.drawText(xs[i] - 6, 62, "+!", Color565::Lime);
      }
      gfx.drawText(xs[i] - (i == 1 ? 12 : 3), 102, keys[i], Color565::White);
    }

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
