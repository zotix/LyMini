#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameFlappy : public IGame {
private:
  float _birdY, _birdVY;
  struct Pipe { float x; int16_t gapY; bool passed; };
  Pipe _pipes[3];
  float _speed;

  void resetPipe(int i, float startX) {
    _pipes[i].x = startX;
    _pipes[i].gapY = random(34, 88);
    _pipes[i].passed = false;
  }

public:
  const char* id() const override { return "flappy"; }
  const char* title() const override { return "Flappy Bird"; }
  const char* desc() const override { return "Flap through pipes!"; }
  uint16_t themeColor() const override { return Color565::Gold; }

  void init() override {
    _birdY = 64.0f;
    _birdVY = 0.0f;
    _speed = 46.0f;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    for (int i = 0; i < 3; ++i) resetPipe(i, 128.0f + i * 54.0f);
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    if (input.leftPressed || input.rightPressed || input.okPressed) {
      _birdVY = -112.0f;
    }

    _birdVY += 290.0f * dt;
    if (_birdVY > 140.0f) _birdVY = 140.0f;
    _birdY += _birdVY * dt;

    if (_birdY < 16.0f) { _birdY = 16.0f; _birdVY = 0; }
    if (_birdY > 116.0f) { triggerGameOver(); return; }

    for (int i = 0; i < 3; ++i) {
      _pipes[i].x -= _speed * dt;
      if (!_pipes[i].passed && _pipes[i].x + 14 < 28) {
        _pipes[i].passed = true;
        _score++;
        if (_score % 5 == 0 && _speed < 82.0f) _speed += 4.0f;
      }
      if (_pipes[i].x < -16.0f) {
        float maxX = 0;
        for (int j = 0; j < 3; ++j) if (_pipes[j].x > maxX) maxX = _pipes[j].x;
        resetPipe(i, maxX + 54.0f);
      }

      // Collision with bird at (28, _birdY), radius ~4
      if (28 + 4 > _pipes[i].x && 28 - 4 < _pipes[i].x + 14) {
        if (_birdY - 4 < _pipes[i].gapY - 19 || _birdY + 4 > _pipes[i].gapY + 19) {
          triggerGameOver();
          return;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(12, 28, 56));
    // Ground
    gfx.fillRect(0, 120, SCREEN_WIDTH, 8, Color565::Emerald);
    gfx.drawFastHLine(0, 120, SCREEN_WIDTH, Color565::Lime);

    for (int i = 0; i < 3; ++i) {
      int16_t px = (int16_t)_pipes[i].x;
      int16_t topH = (_pipes[i].gapY - 19) - 13;
      if (topH > 0) {
        gfx.fillRect(px, 13, 14, topH, Color565::Green);
        gfx.drawRect(px, 13, 14, topH, Color565::Lime);
      }
      int16_t botY = _pipes[i].gapY + 19;
      int16_t botH = 120 - botY;
      if (botH > 0) {
        gfx.fillRect(px, botY, 14, botH, Color565::Green);
        gfx.drawRect(px, botY, 14, botH, Color565::Lime);
      }
    }

    // Bird
    int16_t by = (int16_t)_birdY;
    gfx.fillCircle(28, by, 5, Color565::Gold);
    gfx.fillCircle(30, by - 1, 1, Color565::Black);
    gfx.fillTriangle(32, by, 36, by + 1, 32, by + 3, Color565::Orange);

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
