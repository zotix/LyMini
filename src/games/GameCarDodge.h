#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameCarDodge : public IGame {
private:
  uint8_t _playerLane; // 0: Left, 1: Middle, 2: Right
  float _playerX;
  float _roadOffset;
  float _speed;
  bool _nitro;

  struct TrafficCar {
    uint8_t lane;
    float y;
    float speed;
    uint16_t color;
    bool active;
  };
  TrafficCar _traffic[4];

  struct Coin {
    uint8_t lane;
    float y;
    bool active;
  };
  Coin _coins[2];

  float getLaneX(uint8_t lane) {
    if (lane == 0) return 30.0f;
    if (lane == 1) return 56.0f;
    return 82.0f;
  }

public:
  const char* id() const override { return "cardodge"; }
  const char* title() const override { return "Car Dodge"; }
  const char* desc() const override { return "3-Lane Turbo Highway!"; }
  uint16_t themeColor() const override { return Color565::Orange; }

  void init() override {
    _playerLane = 1;
    _playerX = getLaneX(1);
    _roadOffset = 0.0f;
    _speed = 90.0f;
    _nitro = false;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    for (int i = 0; i < 4; ++i) {
      _traffic[i].active = false;
    }
    for (int i = 0; i < 2; ++i) {
      _coins[i].active = false;
    }
  }

  void spawnTraffic(int i) {
    _traffic[i].lane = random(0, 3);
    _traffic[i].y = -random(30, 70);
    _traffic[i].speed = random(30, 60) * 1.0f;
    uint16_t colors[4] = { Color565::Blue, Color565::Yellow, Color565::Magenta, Color565::White };
    _traffic[i].color = colors[random(0, 4)];
    _traffic[i].active = true;
  }

  void spawnCoin(int i) {
    _coins[i].lane = random(0, 3);
    _coins[i].y = -random(40, 90);
    _coins[i].active = true;
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Lane Switching
    if (input.leftPressed && _playerLane > 0) {
      _playerLane--;
    }
    if (input.rightPressed && _playerLane < 2) {
      _playerLane++;
    }

    // Nitro Boost (Hold OK)
    _nitro = input.okHeld;
    float currentSpeed = _nitro ? (_speed * 1.6f) : _speed;
    _speed = 90.0f + sqrtf(_score) * 1.8f;
    if (_speed > 180.0f) _speed = 180.0f;

    // Smooth Car X position transition
    float targetX = getLaneX(_playerLane);
    _playerX += (targetX - _playerX) * 14.0f * dt;

    // Road scrolling
    _roadOffset += currentSpeed * dt;
    if (_roadOffset >= 16.0f) _roadOffset -= 16.0f;

    // Traffic Cars
    for (int i = 0; i < 4; ++i) {
      if (!_traffic[i].active) {
        if (random(0, 100) < 4) spawnTraffic(i);
      } else {
        _traffic[i].y += (currentSpeed - _traffic[i].speed) * dt;

        if (_traffic[i].y > SCREEN_HEIGHT + 20) {
          _traffic[i].active = false;
          _score += _nitro ? 20 : 10;
        }

        // Collision Check with Player (Player Y = 104, H = 18, W = 14)
        float tx = getLaneX(_traffic[i].lane);
        if (_playerX < tx + 14.0f && _playerX + 14.0f > tx &&
            104.0f < _traffic[i].y + 18.0f && 122.0f > _traffic[i].y) {
          triggerGameOver();
          return;
        }
      }
    }

    // Coins
    for (int i = 0; i < 2; ++i) {
      if (!_coins[i].active) {
        if (random(0, 100) < 3) spawnCoin(i);
      } else {
        _coins[i].y += currentSpeed * dt;
        if (_coins[i].y > SCREEN_HEIGHT + 10) {
          _coins[i].active = false;
        }

        // Collect Coin
        float cx = getLaneX(_coins[i].lane);
        if (_playerX < cx + 12.0f && _playerX + 14.0f > cx &&
            104.0f < _coins[i].y + 8.0f && 122.0f > _coins[i].y) {
          _coins[i].active = false;
          _score += 50;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Grass/Sidewalk Borders
    gfx.fillRect(0, 12, 18, SCREEN_HEIGHT - 12, Color565::rgb(20, 80, 20));
    gfx.fillRect(SCREEN_WIDTH - 18, 12, 18, SCREEN_HEIGHT - 12, Color565::rgb(20, 80, 20));

    // Road (Asphalt Dark Gray)
    gfx.fillRect(18, 12, SCREEN_WIDTH - 36, SCREEN_HEIGHT - 12, Color565::rgb(40, 40, 45));

    // Red-White Curbs
    int16_t curbOffset = (int16_t)_roadOffset % 16;
    for (int16_t y = 12 - curbOffset; y < SCREEN_HEIGHT; y += 16) {
      uint16_t c1 = ((y / 16) % 2 == 0) ? Color565::Red : Color565::White;
      gfx.fillRect(18, y, 2, 8, c1);
      gfx.fillRect(SCREEN_WIDTH - 20, y, 2, 8, c1);
    }

    // Lane Dividers (Dashed Yellow/White)
    int16_t dashOffset = (int16_t)_roadOffset % 16;
    for (int16_t y = 12 - dashOffset; y < SCREEN_HEIGHT; y += 16) {
      gfx.drawFastVLine(48, y, 8, Color565::White);
      gfx.drawFastVLine(78, y, 8, Color565::White);
    }

    // Coins
    for (int i = 0; i < 2; ++i) {
      if (_coins[i].active) {
        float cx = getLaneX(_coins[i].lane) + 4.0f;
        gfx.fillCircle((int16_t)cx, (int16_t)_coins[i].y, 3, Color565::Gold);
        gfx.drawPixel((int16_t)cx, (int16_t)_coins[i].y, Color565::Yellow);
      }
    }

    // Traffic Cars
    for (int i = 0; i < 4; ++i) {
      if (_traffic[i].active) {
        int16_t tx = (int16_t)getLaneX(_traffic[i].lane);
        int16_t ty = (int16_t)_traffic[i].y;
        // Car Body
        gfx.fillRoundRect(tx, ty, 14, 18, 3, _traffic[i].color);
        // Windshield
        gfx.fillRect(tx + 2, ty + 4, 10, 4, Color565::Cyan);
        // Taillights
        gfx.fillRect(tx + 2, ty + 16, 2, 2, Color565::Red);
        gfx.fillRect(tx + 10, ty + 16, 2, 2, Color565::Red);
      }
    }

    // Player Car (Bright Red Sports Car)
    int16_t px = (int16_t)_playerX;
    int16_t py = 104;
    gfx.fillRoundRect(px, py, 14, 18, 3, Color565::Red);
    // Windshield (Black/Cyan)
    gfx.fillRect(px + 2, py + 5, 10, 5, Color565::DeepBlue);
    // Headlights
    gfx.fillRect(px + 2, py, 2, 2, Color565::Yellow);
    gfx.fillRect(px + 10, py, 2, 2, Color565::Yellow);
    // Wheels
    gfx.fillRect(px - 1, py + 2, 1, 4, Color565::Black);
    gfx.fillRect(px + 14, py + 2, 1, 4, Color565::Black);
    gfx.fillRect(px - 1, py + 12, 1, 4, Color565::Black);
    gfx.fillRect(px + 14, py + 12, 1, 4, Color565::Black);

    // Nitro Flames
    if (_nitro) {
      gfx.fillRect(px + 4, py + 18, 2, 4, Color565::Orange);
      gfx.fillRect(px + 8, py + 18, 2, 4, Color565::Orange);
      gfx.fillRect(px + 5, py + 22, 1, 3, Color565::Yellow);
      gfx.fillRect(px + 9, py + 22, 1, 3, Color565::Yellow);
    }

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
