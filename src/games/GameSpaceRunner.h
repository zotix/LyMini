#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameSpaceRunner : public IGame {
private:
  float _shipX;
  float _bulletTimer;

  struct Bullet {
    float x, y;
    bool active;
  };
  Bullet _bullets[8];

  struct Enemy {
    float x, y;
    float vx, vy;
    uint8_t hp;
    uint8_t type; // 0: Meteor, 1: Alien Drone
    bool active;
  };
  Enemy _enemies[6];

  struct Star {
    float x, y;
    float speed;
    uint16_t color;
  };
  Star _stars[16];

  uint8_t _bombs;

public:
  const char* id() const override { return "space"; }
  const char* title() const override { return "Space Runner"; }
  const char* desc() const override { return "Shoot & Dodge!"; }
  uint16_t themeColor() const override { return Color565::Magenta; }

  void init() override {
    _shipX = 64.0f - 6.0f;
    _score = 0;
    _bombs = 3; // 3 starting bombs for easier gameplay
    _bulletTimer = 0.0f;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    for (int i = 0; i < 8; ++i) _bullets[i].active = false;
    for (int i = 0; i < 6; ++i) _enemies[i].active = false;

    for (int i = 0; i < 16; ++i) {
      _stars[i].x = random(0, SCREEN_WIDTH);
      _stars[i].y = random(12, SCREEN_HEIGHT);
      _stars[i].speed = random(20, 90) * 1.0f;
      _stars[i].color = (_stars[i].speed > 60) ? Color565::White : Color565::GrayMid;
    }
  }

  void spawnEnemy(int i) {
    _enemies[i].x = random(10, SCREEN_WIDTH - 20);
    _enemies[i].y = -random(10, 30);
    _enemies[i].type = (random(0, 4) == 0 ? 1 : 0);
    _enemies[i].hp = 1; // 1-hit destroy for easier shooting
    _enemies[i].vy = random(24, 52) * 1.0f;
    _enemies[i].vx = (_enemies[i].type == 1) ? ((random(0, 2) == 0 ? -1 : 1) * 14.0f) : 0;
    _enemies[i].active = true;
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Move Stars
    for (int i = 0; i < 16; ++i) {
      _stars[i].y += _stars[i].speed * dt;
      if (_stars[i].y >= SCREEN_HEIGHT) {
        _stars[i].y = 12;
        _stars[i].x = random(0, SCREEN_WIDTH);
      }
    }

    // Ship Movement
    float shipSpeed = 110.0f;
    if (input.leftHeld) _shipX -= shipSpeed * dt;
    if (input.rightHeld) _shipX += shipSpeed * dt;
    if (_shipX < 4.0f) _shipX = 4.0f;
    if (_shipX > SCREEN_WIDTH - 16.0f) _shipX = SCREEN_WIDTH - 16.0f;

    // Auto-fire laser bullets (faster fire rate)
    _bulletTimer += dt;
    if (_bulletTimer >= 0.12f) {
      _bulletTimer = 0.0f;
      for (int i = 0; i < 8; ++i) {
        if (!_bullets[i].active) {
          _bullets[i].x = _shipX + 5.0f;
          _bullets[i].y = 108.0f;
          _bullets[i].active = true;
          break;
        }
      }
    }

    // Screen Bomb (OK Short Released so holding BOOT to exit doesn't trigger bomb)
    if (input.okShortReleased && _bombs > 0) {
      _bombs--;
      for (int i = 0; i < 6; ++i) {
        if (_enemies[i].active) {
          _enemies[i].active = false;
          _score += 30;
        }
      }
    }

    // Move Bullets
    for (int i = 0; i < 8; ++i) {
      if (_bullets[i].active) {
        _bullets[i].y -= 210.0f * dt;
        if (_bullets[i].y < 12.0f) {
          _bullets[i].active = false;
        }
      }
    }

    // Move Enemies
    for (int i = 0; i < 6; ++i) {
      if (!_enemies[i].active) {
        if (random(0, 100) < 4) spawnEnemy(i);
      } else {
        _enemies[i].y += _enemies[i].vy * dt;
        _enemies[i].x += _enemies[i].vx * dt;

        if (_enemies[i].x < 4.0f || _enemies[i].x > SCREEN_WIDTH - 16.0f) {
          _enemies[i].vx = -_enemies[i].vx;
        }

        if (_enemies[i].y > SCREEN_HEIGHT + 10.0f) {
          _enemies[i].active = false;
        }

        // Bullet vs Enemy Collision (wider hitbox)
        for (int b = 0; b < 8; ++b) {
          if (_bullets[b].active) {
            if (_bullets[b].x >= _enemies[i].x - 3.0f && _bullets[b].x <= _enemies[i].x + 12.0f &&
                _bullets[b].y >= _enemies[i].y - 2.0f && _bullets[b].y <= _enemies[i].y + 12.0f) {
              _bullets[b].active = false;
              _enemies[i].hp--;
              if (_enemies[i].hp == 0) {
                _enemies[i].active = false;
                _score += (_enemies[i].type == 1) ? 35 : 20;
              }
              break;
            }
          }
        }

        // Enemy vs Ship Collision (forgiving hitbox)
        if (_enemies[i].active) {
          if (_shipX + 2.0f < _enemies[i].x + 8.0f && _shipX + 9.0f > _enemies[i].x + 1.0f &&
              112.0f < _enemies[i].y + 8.0f && 121.0f > _enemies[i].y + 1.0f) {
            triggerGameOver();
            return;
          }
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Deep Space Background
    gfx.fillRect(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, Color565::Black);

    // Stars
    for (int i = 0; i < 16; ++i) {
      gfx.drawPixel((int16_t)_stars[i].x, (int16_t)_stars[i].y, _stars[i].color);
    }

    // Bullets (Neon Cyan beam)
    for (int i = 0; i < 8; ++i) {
      if (_bullets[i].active) {
        gfx.fillRect((int16_t)_bullets[i].x, (int16_t)_bullets[i].y, 2, 5, Color565::NeonCyan);
      }
    }

    // Enemies
    for (int i = 0; i < 6; ++i) {
      if (!_enemies[i].active) continue;
      int16_t ex = (int16_t)_enemies[i].x;
      int16_t ey = (int16_t)_enemies[i].y;

      if (_enemies[i].type == 0) {
        // Meteor (Orange/Coral Rock)
        gfx.fillRoundRect(ex, ey, 9, 9, 2, Color565::Coral);
        gfx.drawPixel(ex + 2, ey + 3, Color565::Orange);
      } else {
        // Alien Drone (Violet Ship)
        gfx.fillTriangle(ex, ey + 2, ex + 10, ey + 2, ex + 5, ey + 9, Color565::Violet);
        gfx.fillRect(ex + 3, ey, 5, 3, Color565::Magenta);
        gfx.drawPixel(ex + 5, ey + 4, Color565::Yellow);
      }
    }

    // Player Ship (Sleek Fighter)
    int16_t sx = (int16_t)_shipX;
    gfx.fillTriangle(sx + 5, 110, sx, 122, sx + 11, 122, Color565::SkyBlue);
    gfx.fillRect(sx + 4, 113, 3, 6, Color565::White);
    // Engine Thrusters
    gfx.fillRect(sx + 2, 123, 2, 3, Color565::Orange);
    gfx.fillRect(sx + 7, 123, 2, 3, Color565::Orange);

    drawHud(gfx);

    // Bomb Icons on HUD
    for (int i = 0; i < _bombs; ++i) {
      gfx.fillCircle(SCREEN_WIDTH / 2 - 10 + (i * 8), 6, 2, Color565::Gold);
    }

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
