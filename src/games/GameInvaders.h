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
  uint32_t _aliens[3];
  uint8_t _shields[3]; // 0..3 HP for 3 shields
  float _fleetX, _fleetY, _fleetDir;
  float _animT, _autoFireT;
  uint8_t _wave, _lives;

  void resetWave() {
    for (int r = 0; r < 3; ++r) _aliens[r] = 0x7F;
    for (int s = 0; s < 3; ++s) _shields[s] = 3;
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
    _lives = 3;
    _score = 0;
    _animT = 0;
    _autoFireT = 0;
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

    _animT += dt;
    _autoFireT += dt;
    if (input.leftHeld) _playerX -= 92.0f * dt;
    if (input.rightHeld) _playerX += 92.0f * dt;
    _playerX = constrain(_playerX, 8.0f, 120.0f);

    if (!_bulletActive && (input.okPressed || _autoFireT >= 0.35f)) {
      _autoFireT = 0;
      _bulletActive = true;
      _bulletX = _playerX;
      _bulletY = 110.0f;
    }

    if (_bulletActive) {
      _bulletY -= 165.0f * dt;
      if (_bulletY < 14.0f) _bulletActive = false;
    }

    if (_eBulletActive) {
      _eBulletY += 54.0f * dt;
      if (_eBulletY > 128.0f) _eBulletActive = false;
      else {
        // Check shields at Y=102 (X=22, 64, 106)
        const int16_t sx[3] = { 22, 64, 106 };
        for (int s = 0; s < 3; ++s) {
          if (_shields[s] > 0 && fabsf(_eBulletX - sx[s]) < 9.0f && fabsf(_eBulletY - 102.0f) < 4.0f) {
            _shields[s]--;
            _eBulletActive = false;
            break;
          }
        }
        if (_eBulletActive && fabsf(_eBulletX - _playerX) < 6.0f && fabsf(_eBulletY - 115.0f) < 5.0f) {
          _eBulletActive = false;
          _lives--;
          if (_lives == 0) { triggerGameOver(); return; }
        }
      }
    }

    float spd = 12.0f + _wave * 3.0f;
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
        if (ay >= 106.0f) { triggerGameOver(); return; }
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
      _fleetY += 5.0f;
    }

    if (!_eBulletActive && random(0, 65) == 0) {
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
    gfx.fillGradientV(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(4, 6, 18), Color565::rgb(12, 16, 36));
    // Twinkling stars
    for (int i = 0; i < 14; ++i) {
      int16_t sx = (i * 37 + 11) % 124 + 2;
      int16_t sy = (i * 23 + 17) % 100 + 16;
      gfx.drawPixel(sx, sy, ((int)(_animT * 3 + i) & 1) ? Color565::GrayMid : Color565::White);
    }

    bool legToggle = ((int)(_animT * 4) & 1) != 0;
    uint16_t cols[3] = { Color565::Magenta, Color565::NeonCyan, Color565::Lime };
    for (int r = 0; r < 3; ++r) {
      for (int c = 0; c < 7; ++c) {
        if (!(_aliens[r] & (1 << c))) continue;
        int16_t ax = (int16_t)(_fleetX + c * 14.0f);
        int16_t ay = (int16_t)(_fleetY + r * 12.0f);
        gfx.fillRoundRect(ax + 1, ay + 1, 8, 5, 2, cols[r]);
        gfx.drawPixel(ax + 3, ay + 3, Color565::Black);
        gfx.drawPixel(ax + 6, ay + 3, Color565::Black);
        gfx.drawPixel(ax + (legToggle ? 1 : 2), ay + 6, cols[r]);
        gfx.drawPixel(ax + (legToggle ? 8 : 7), ay + 6, cols[r]);
      }
    }

    // Shields
    const int16_t sx[3] = { 22, 64, 106 };
    for (int s = 0; s < 3; ++s) {
      if (_shields[s] == 0) continue;
      uint16_t sc = (_shields[s] == 3) ? Color565::Emerald : (_shields[s] == 2 ? Color565::Gold : Color565::Orange);
      gfx.fillRoundRect(sx[s] - 8, 100, 16, 5, 2, sc);
    }

    if (_bulletActive) gfx.fillRect((int16_t)_bulletX - 1, (int16_t)_bulletY, 2, 6, Color565::Gold);
    if (_eBulletActive) gfx.fillCircle((int16_t)_eBulletX, (int16_t)_eBulletY, 2, Color565::Red);

    // Player Jet + Thruster
    int16_t px = (int16_t)_playerX;
    gfx.fillTriangle(px, 111, px - 7, 119, px + 7, 119, Color565::NeonCyan);
    gfx.fillRect(px - 2, 113, 4, 4, Color565::White);
    if (legToggle) gfx.fillTriangle(px - 2, 119, px + 2, 119, px, 123, Color565::Orange);

    // Lives indicator bottom-left
    for (int i = 0; i < _lives; ++i) {
      gfx.fillCircle(6 + i * 7, 123, 2, Color565::Red);
    }

    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
