#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameFrogger : public IGame {
private:
  int16_t _frogX;
  int8_t _frogLane; // 0=start(bottom) .. 8=goal(top)
  struct LaneObj { float x; uint8_t w; float speed; bool isLog; };
  LaneObj _lanes[7]; // lanes 1..7

  void initLanes(float mult) {
    // 1..3 = road cars, 4 = safe median, 5..7 = river logs
    _lanes[0] = { 10, 20,  34.0f * mult, false };
    _lanes[1] = { 80, 24, -42.0f * mult, false };
    _lanes[2] = { 40, 18,  48.0f * mult, false };
    _lanes[3] = {  0,  0,   0.0f,        false }; // safe strip
    _lanes[4] = { 20, 38,  28.0f * mult, true  };
    _lanes[5] = { 60, 34, -34.0f * mult, true  };
    _lanes[6] = { 15, 42,  26.0f * mult, true  };
  }

public:
  const char* id() const override { return "frogger"; }
  const char* title() const override { return "Frogger"; }
  const char* desc() const override { return "Cross road & river!"; }
  uint16_t themeColor() const override { return Color565::Green; }

  void init() override {
    _frogX = 64;
    _frogLane = 0;
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
        initLanes(1.0f + min(1.0f, _score * 0.002f));
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
                   (_frogX + 4 > L.x + 70 && _frogX - 4 < L.x + 70 + L.w);
      if (L.isLog) {
        if (!onObj) { triggerGameOver(); return; }
        _frogX += (int16_t)(L.speed * dt);
        if (_frogX < 2 || _frogX > 126) { triggerGameOver(); return; }
      } else {
        if (onObj) { triggerGameOver(); return; }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, 12, Color565::Emerald); // Goal
    gfx.fillRect(0, 25, SCREEN_WIDTH, 36, Color565::DeepBlue); // River
    gfx.fillRect(0, 61, SCREEN_WIDTH, 12, Color565::rgb(35, 65, 45)); // Safe median
    gfx.fillRect(0, 73, SCREEN_WIDTH, 36, Color565::rgb(32, 32, 38)); // Road
    gfx.fillRect(0, 109, SCREEN_WIDTH, 19, Color565::rgb(24, 52, 24)); // Start

    for (int i = 0; i < 7; ++i) {
      if (_lanes[i].w == 0) continue;
      int16_t y = 109 - (i + 1) * 12 + 2;
      uint16_t c = _lanes[i].isLog ? Color565::Orange : Color565::Red;
      gfx.fillRoundRect((int16_t)_lanes[i].x, y, _lanes[i].w, 8, 2, c);
      gfx.fillRoundRect((int16_t)_lanes[i].x + 70, y, _lanes[i].w, 8, 2, c);
    }

    int16_t fy = 109 - _frogLane * 12 + 6;
    gfx.fillCircle(_frogX, fy, 4, Color565::Lime);
    gfx.drawPixel(_frogX - 2, fy - 2, Color565::Black);
    gfx.drawPixel(_frogX + 2, fy - 2, Color565::Black);

    gfx.drawCenteredText(118, "A:L  B:R  BOOT:HOP", Color565::Gold);
    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
