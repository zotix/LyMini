#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameInvaders : public IGame {
private:
  float _playerX;
  float _bulletX, _bulletY;
  bool _bulletActive;
  float _eBulletX, _eBulletY;
  bool _eBulletActive;
  uint32_t _aliens[3]; // bitmask 7 cols per row
  float _fleetX, _fleetY, _fleetDir;
  float _stepTimer;
  uint8_t _wave;

  void resetWave() {
    for (int r = 0; r < 3; ++r) _aliens[r] = 0x7F; // 7 aliens per row
    _fleetX = 14.0f;
    _fleetY = 20.0f;
    _fleetDir = 1.0f;
  }

public:
  const char* id() const override { return "invaders"; }
  const char* title() const override { return "Space Invader"; }
  const char* desc() const override { return "Defend Earth!"; }
  uint16_t themeColor() const override { return Color565::Lime; }

  void init() override {
    _playerX = 64.0f;
    _bulletActive = false;
    _eBulletActive = false;
    _wave = 1;
    _score = 0;
    _stepTimer = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    resetWave();
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    if (input.leftHeld) _playerX -= 84.0f * dt;
    if (input.rightHeld) _playerX += 84.0f * dt;
    _playerX = constrain(_playerX, 8.0f, 120.0f);

    if (!_bulletActive && (input.okPressed || input.leftPressed || input.rightPressed)) {
      _bulletActive = true;
      _bulletX = _playerX;
      _bulletY = 112.0f;
    }

    if (_bulletActive) {
      _bulletY -= 150.0f * dt;
      if (_bulletY < 14.0f) _bulletActive = false;
    }

    if (_eBulletActive) {
      _eBulletY += 75.0f * dt;
      if (_eBulletY > 128.0f) _eBulletActive = false;
      else if (fabsf(_eBulletX - _playerX) < 6.0f && fabsf(_eBulletY - 116.0f) < 5.0f) {
        triggerGameOver();
        return;
      }
    }

    float spd = 16.0f + _wave * 4.0f;
    _fleetX += _fleetDir * spd * dt;
    bool hitEdge = false;
    int alive = 0;
    for (int r = 0; r < 3; ++r) {
      for (int c = 0; c < 7; ++c) {
        if (!(_aliens[r] & (1 << c))) continue;
        alive++;
        float ax = _fleetX + c * 14.0f;
        float ay = _fleetY + r * 12.0f;
        if (ax < 6.0f || ax > 114.0f) hitEdge = true;
        if (ay >= 108.0f) { triggerGameOver(); return; }
        if (_bulletActive && fabsf(_bulletX - (ax + 4)) < 6.0f && fabsf(_bulletY - (ay + 4)) < 6.0f) {
          _aliens[r] &= ~(1 << c);
          _bulletActive = false;
          _score += (3 - r) * 10;
        }
      }
    }

    if (alive == 0) {
      _wave++;
      _score += 50;
      resetWave();
      return;
    }
    if (hitEdge) {
      _fleetDir = -_fleetDir;
      _fleetX += _fleetDir * 3.0f;
      _fleetY += 6.0f;
    }

    if (!_eBulletActive && random(0, 45) == 0) {
      int c = random(0, 7);
      for (int r = 2; r >= 0; --r) {
        if (_aliens[r] & (1 << c)) {
          _eBulletActive = true;
          _eBulletX = _fleetX + c * 14.0f + 4;
          _eBulletY = _fleetY + r * 12.0f + 8;
          break;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::Black);
    uint16_t cols[3] = { Color565::Magenta, Color565::Cyan, Color565::Lime };
    for (int r = 0; r < 3; ++r) {
      for (int c = 0; c < 7; ++c) {
        if (!(_aliens[r] & (1 << c))) continue;
        int16_t ax = (int16_t)(_fleetX + c * 14.0f);
        int16_t ay = (int16_t)(_fleetY + r * 12.0f);
        gfx.fillRect(ax + 1, ay + 1, 7, 5, cols[r]);
        gfx.drawPixel(ax + 2, ay + 3, Color565::Black);
        gfx.drawPixel(ax + 6, ay + 3, Color565::Black);
      }
    }

    if (_bulletActive) gfx.fillRect((int16_t)_bulletX - 1, (int16_t)_bulletY, 2, 5, Color565::Gold);
    if (_eBulletActive) gfx.fillRect((int16_t)_eBulletX - 1, (int16_t)_eBulletY, 2, 5, Color565::Red);

    int16_t px = (int16_t)_playerX;
    gfx.fillRect(px - 6, 116, 12, 5, Color565::Lime);
    gfx.fillRect(px - 2, 113, 4, 3, Color565::Lime);

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
