#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameStack : public IGame {
private:
  struct Block { int16_t x; int16_t w; uint16_t col; };
  Block _stack[12];
  uint8_t _count;
  float _curX, _curW, _dir, _speed;

  uint16_t levelColor(uint8_t lvl) const {
    const uint16_t cols[6] = {
      Color565::NeonCyan, Color565::Lime, Color565::Gold,
      Color565::Orange, Color565::Magenta, Color565::SkyBlue
    };
    return cols[lvl % 6];
  }

public:
  const char* id() const override { return "stack"; }
  const char* title() const override { return "Tower Stack"; }
  const char* desc() const override { return "Drop blocks to stack!"; }
  uint16_t themeColor() const override { return Color565::Magenta; }

  void init() override {
    _count = 1;
    _stack[0] = { 32, 64, levelColor(0) };
    _curW = 64.0f;
    _curX = 4.0f;
    _dir = 1.0f;
    _speed = 62.0f;
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

    _curX += _dir * _speed * dt;
    if (_curX <= 2.0f) { _curX = 2.0f; _dir = 1.0f; }
    if (_curX + _curW >= 126.0f) { _curX = 126.0f - _curW; _dir = -1.0f; }

    if (input.leftPressed || input.rightPressed || input.okPressed) {
      Block& prev = _stack[_count - 1];
      int16_t ix = (int16_t)_curX;
      int16_t iw = (int16_t)_curW;
      int16_t left = max(ix, prev.x);
      int16_t right = min((int16_t)(ix + iw), (int16_t)(prev.x + prev.w));
      int16_t overlap = right - left;
      if (overlap <= 0) {
        triggerGameOver();
        return;
      }

      _score += 10;
      if (abs(ix - prev.x) <= 2) {
        left = prev.x;
        overlap = prev.w;
        _score += 5; // Perfect drop bonus
      }

      if (_count >= 12) {
        for (int i = 0; i < 11; ++i) _stack[i] = _stack[i + 1];
        _count = 11;
      }
      _stack[_count++] = { left, overlap, levelColor(_score / 10) };
      _curW = (float)overlap;
      _curX = (_dir > 0) ? 2.0f : (126.0f - _curW);
      if (_speed < 135.0f) _speed += 3.0f;
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(12, 14, 28));
    for (int i = 0; i < _count; ++i) {
      int16_t y = 116 - i * 8;
      gfx.fillRect(_stack[i].x, y, _stack[i].w, 7, _stack[i].col);
      gfx.drawRect(_stack[i].x, y, _stack[i].w, 7, Color565::White);
    }

    int16_t curY = 116 - _count * 8;
    gfx.fillRect((int16_t)_curX, curY, (int16_t)_curW, 7, levelColor((_score / 10) + 1));

    gfx.drawCenteredText(16, "PRESS ANY KEY TO DROP", Color565::GrayLight);
    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
