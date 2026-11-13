#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameFlappy : public IGame {
private:
  float _birdY, _birdVY, _cloudX;
  struct Pipe { float x; int16_t gapY; bool passed; };
  Pipe _pipes[3];
  float _speed;

  void resetPipe(int i, float startX) {
    _pipes[i].x = startX;
    _pipes[i].gapY = random(38, 86);
    _pipes[i].passed = false;
  }

public:
  const char* id() const override { return "flappy"; }
  const char* title() const override { return "Flappy Bird"; }
  const char* desc() const override { return "Flap through pipes!"; }
  uint16_t themeColor() const override { return Color565::Gold; }

  void init() override {
    _birdY = 60.0f;
    _birdVY = 0.0f;
    _cloudX = 20.0f;
    _speed = 36.0f;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    for (int i = 0; i < 3; ++i) resetPipe(i, 130.0f + i * 62.0f);
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    if (input.leftPressed || input.rightPressed || input.okPressed) {
      _birdVY = -96.0f;
    }

    _birdVY += 225.0f * dt;
    if (_birdVY > 125.0f) _birdVY = 125.0f;
    _birdY += _birdVY * dt;

    _cloudX -= 10.0f * dt;
    if (_cloudX < -30.0f) _cloudX = 135.0f;

    if (_birdY < 17.0f) { _birdY = 17.0f; _birdVY = 0; }
    if (_birdY > 114.0f) { triggerGameOver(); return; }

    for (int i = 0; i < 3; ++i) {
      _pipes[i].x -= _speed * dt;
      if (!_pipes[i].passed && _pipes[i].x + 16 < 28) {
        _pipes[i].passed = true;
        _score++;
        if (_score % 6 == 0 && _speed < 66.0f) _speed += 3.0f;
      }
      if (_pipes[i].x < -18.0f) {
        float maxX = 0;
        for (int j = 0; j < 3; ++j) if (_pipes[j].x > maxX) maxX = _pipes[j].x;
        resetPipe(i, maxX + 62.0f);
      }

      // Generous gap (±24px) & forgiving hitbox
      if (28 + 3 > _pipes[i].x && 28 - 3 < _pipes[i].x + 16) {
        if (_birdY - 3 < _pipes[i].gapY - 24 || _birdY + 3 > _pipes[i].gapY + 24) {
          triggerGameOver();
          return;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillGradientV(0, 13, SCREEN_WIDTH, 105, Color565::rgb(26, 82, 156), Color565::rgb(104, 182, 232));

    // Fluffy clouds
    int16_t cx = (int16_t)_cloudX;
    gfx.fillCircle(cx, 30, 7, Color565::White);
    gfx.fillCircle(cx + 8, 28, 9, Color565::White);
    gfx.fillCircle(cx + 16, 31, 6, Color565::White);

    int16_t cx2 = (cx + 68) % 160 - 20;
    gfx.fillCircle(cx2, 44, 5, Color565::rgb(230, 242, 255));
    gfx.fillCircle(cx2 + 6, 43, 6, Color565::rgb(230, 242, 255));

    // 3D Highlighted Pipes
    for (int i = 0; i < 3; ++i) {
      int16_t px = (int16_t)_pipes[i].x;
      int16_t topEnd = _pipes[i].gapY - 24;
      if (topEnd > 13) {
        gfx.fillRect(px + 1, 13, 14, topEnd - 13, Color565::Green);
        gfx.fillRect(px + 3, 13, 3, topEnd - 13, Color565::Lime);
        gfx.fillRect(px - 1, topEnd - 6, 18, 6, Color565::Emerald);
        gfx.drawRect(px - 1, topEnd - 6, 18, 6, Color565::Lime);
      }
      int16_t botY = _pipes[i].gapY + 24;
      int16_t botH = 118 - botY;
      if (botH > 0) {
        gfx.fillRect(px + 1, botY, 14, botH, Color565::Green);
        gfx.fillRect(px + 3, botY, 3, botH, Color565::Lime);
        gfx.fillRect(px - 1, botY, 18, 6, Color565::Emerald);
        gfx.drawRect(px - 1, botY, 18, 6, Color565::Lime);
      }
    }

    // Checkered grass ground
    gfx.fillRect(0, 118, SCREEN_WIDTH, 10, Color565::rgb(138, 92, 46));
    gfx.fillRect(0, 118, SCREEN_WIDTH, 4, Color565::Lime);
    for (int gx = 0; gx < SCREEN_WIDTH; gx += 12) {
      gfx.fillRect(gx, 118, 6, 4, Color565::Green);
    }

    // Detailed Bird with flapping wing
    int16_t by = (int16_t)_birdY;
    gfx.fillCircle(28, by, 6, Color565::Gold);
    gfx.drawCircle(28, by, 6, Color565::Orange);
    int16_t wingY = (_birdVY < 0) ? (by - 2) : (by + 1);
    gfx.fillRoundRect(23, wingY, 6, 4, 2, Color565::White);
    gfx.fillCircle(31, by - 2, 2, Color565::White);
    gfx.drawPixel(32, by - 2, Color565::Black);
    gfx.fillTriangle(34, by, 39, by + 1, 34, by + 3, Color565::Red);

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
