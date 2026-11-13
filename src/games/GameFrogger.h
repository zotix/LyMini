#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameFrogger : public IGame {
private:
  int16_t _frogX;
  int8_t _frogLane; // 0=start(bottom) .. 8=goal(top)
  uint8_t _lives;
  struct LaneObj { float x; uint8_t w; float speed; bool isLog; };
  LaneObj _lanes[7];

  void initLanes(float mult) {
    _lanes[0] = { 10, 20,  24.0f * mult, false };
    _lanes[1] = { 76, 22, -28.0f * mult, false };
    _lanes[2] = { 35, 18,  32.0f * mult, false };
    _lanes[3] = {  0,  0,   0.0f,        false };
    _lanes[4] = { 15, 48,  20.0f * mult, true  };
    _lanes[5] = { 55, 46, -24.0f * mult, true  };
    _lanes[6] = { 10, 52,  18.0f * mult, true  };
  }

  void loseLife() {
    _lives--;
    if (_lives == 0) { triggerGameOver(); return; }
    _frogX = 64;
    _frogLane = 0;
  }

public:
  const char* id() const override { return "frogger"; }
  const char* title() const override { return "Frogger"; }
  const char* desc() const override { return "Cross road & river!"; }
  uint16_t themeColor() const override { return Color565::Green; }

  void init() override {
    _frogX = 64;
    _frogLane = 0;
    _lives = 3;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    initLanes(1.0f);
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    if (input.leftPressed) _frogX = max((int16_t)8, (int16_t)(_frogX - 12));
    if (input.rightPressed) _frogX = min((int16_t)120, (int16_t)(_frogX + 12));
    if (input.okPressed) {
      _frogLane++;
      _score += 5;
      if (_frogLane >= 8) {
        _score += 50;
        _frogLane = 0;
        _frogX = 64;
        initLanes(1.0f + min(0.8f, _score * 0.0015f));
        return;
      }
    }

    for (int i = 0; i < 7; ++i) {
      if (_lanes[i].w == 0) continue;
      _lanes[i].x += _lanes[i].speed * dt;
      if (_lanes[i].speed > 0 && _lanes[i].x > 130) _lanes[i].x = -_lanes[i].w;
      if (_lanes[i].speed < 0 && _lanes[i].x + _lanes[i].w < -2) _lanes[i].x = 130;
    }

    if (_frogLane >= 1 && _frogLane <= 7 && _frogLane != 4) {
      LaneObj& L = _lanes[_frogLane - 1];
      bool onObj = (_frogX + 4 > L.x && _frogX - 4 < L.x + L.w) ||
                   (_frogX + 4 > L.x + 74 && _frogX - 4 < L.x + 74 + L.w);
      if (L.isLog) {
        if (!onObj) { loseLife(); return; }
        _frogX += (int16_t)(L.speed * dt);
        if (_frogX < 2 || _frogX > 126) { loseLife(); return; }
      } else {
        if (onObj) { loseLife(); return; }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, 12, Color565::Emerald);
    gfx.fillGradientV(0, 25, SCREEN_WIDTH, 36, Color565::rgb(10, 46, 110), Color565::rgb(16, 72, 150));
    gfx.fillRect(0, 61, SCREEN_WIDTH, 12, Color565::rgb(35, 72, 48));
    gfx.fillRect(0, 73, SCREEN_WIDTH, 36, Color565::rgb(30, 32, 38));
    gfx.fillRect(0, 109, SCREEN_WIDTH, 19, Color565::rgb(24, 56, 26));

    // Road dashed lines
    for (int x = 4; x < SCREEN_WIDTH; x += 16) {
      gfx.drawFastHLine(x, 85, 8, Color565::GrayMid);
      gfx.drawFastHLine(x, 97, 8, Color565::GrayMid);
    }

    for (int i = 0; i < 7; ++i) {
      if (_lanes[i].w == 0) continue;
      int16_t y = 109 - (i + 1) * 12 + 2;
      for (int copy = 0; copy < 2; ++copy) {
        int16_t ox = (int16_t)_lanes[i].x + copy * 74;
        if (_lanes[i].isLog) {
          gfx.fillRoundRect(ox, y, _lanes[i].w, 8, 2, Color565::rgb(148, 86, 38));
          gfx.drawFastHLine(ox + 2, y + 2, _lanes[i].w - 4, Color565::Orange);
        } else {
          uint16_t cc = (i == 1) ? Color565::Yellow : Color565::Red;
          gfx.fillRoundRect(ox, y, _lanes[i].w, 8, 2, cc);
          gfx.fillRect(ox + 4, y + 1, _lanes[i].w - 8, 3, Color565::Cyan);
        }
      }
    }

    // Detailed Frog
    int16_t fy = 109 - _frogLane * 12 + 6;
    gfx.fillCircle(_frogX - 3, fy + 2, 2, Color565::Green);
    gfx.fillCircle(_frogX + 3, fy + 2, 2, Color565::Green);
    gfx.fillCircle(_frogX, fy, 4, Color565::Lime);
    gfx.fillCircle(_frogX - 2, fy - 3, 2, Color565::White);
    gfx.fillCircle(_frogX + 2, fy - 3, 2, Color565::White);
    gfx.drawPixel(_frogX - 2, fy - 3, Color565::Black);
    gfx.drawPixel(_frogX + 2, fy - 3, Color565::Black);

    for (int i = 0; i < _lives; ++i) gfx.fillCircle(6 + i * 7, 123, 2, Color565::Red);
    gfx.drawText(34, 118, "A:L B:R OK:HOP", Color565::Gold);
    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
