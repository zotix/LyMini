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
  uint8_t _lives;
  float _invulnTimer;
  float _spreadTimer;

  struct PowerUp {
    float x, y;
    uint8_t type; // 0: Spread Shot, 1: +1 Bomb
    bool active;
  };
  PowerUp _powerUps[2];

  struct Particle {
    float x, y, vx, vy, life;
    uint16_t color;
  };
  Particle _parts[12];

  void spawnExplosion(float x, float y, uint16_t col) {
    int n = 0;
    for (int i = 0; i < 12 && n < 5; ++i) {
      if (_parts[i].life <= 0.0f) {
        _parts[i].x = x;
        _parts[i].y = y;
        _parts[i].vx = random(-60, 61);
        _parts[i].vy = random(-60, 61);
        _parts[i].life = 0.35f;
        _parts[i].color = col;
        n++;
      }
    }
  }

public:
  const char* id() const override { return "space"; }
  const char* title() const override { return "Space Runner"; }
  const char* desc() const override { return "Shoot & Dodge!"; }
  uint16_t themeColor() const override { return Color565::Magenta; }

  void init() override {
    _shipX = 64.0f - 6.0f;
    _score = 0;
    _bombs = 3; // 3 starting bombs for easier gameplay
    _lives = 3;
    _invulnTimer = 0.0f;
    _spreadTimer = 0.0f;
    _bulletTimer = 0.0f;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    for (int i = 0; i < 8; ++i) _bullets[i].active = false;
    for (int i = 0; i < 6; ++i) _enemies[i].active = false;
    for (int i = 0; i < 2; ++i) _powerUps[i].active = false;
    for (int i = 0; i < 12; ++i) _parts[i].life = 0.0f;

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

    if (_invulnTimer > 0.0f) _invulnTimer -= dt;
    if (_spreadTimer > 0.0f) _spreadTimer -= dt;

    // Update particles
    for (int i = 0; i < 12; ++i) {
      if (_parts[i].life > 0.0f) {
        _parts[i].life -= dt;
        _parts[i].x += _parts[i].vx * dt;
        _parts[i].y += _parts[i].vy * dt;
      }
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

    // Auto-fire laser bullets (single or spread)
    _bulletTimer += dt;
    if (_bulletTimer >= 0.14f) {
      _bulletTimer = 0.0f;
      int toSpawn = (_spreadTimer > 0.0f) ? 2 : 1;
      for (int i = 0; i < 8 && toSpawn > 0; ++i) {
        if (!_bullets[i].active) {
          float offset = (_spreadTimer > 0.0f) ? (toSpawn == 2 ? 1.0f : 9.0f) : 5.0f;
          _bullets[i].x = _shipX + offset;
          _bullets[i].y = 108.0f;
          _bullets[i].active = true;
          toSpawn--;
        }
      }
    }

    // Screen Bomb (OK Short Released so holding BOOT to exit doesn't trigger bomb)
    if (input.okShortReleased && _bombs > 0) {
      _bombs--;
      for (int i = 0; i < 6; ++i) {
        if (_enemies[i].active) {
          spawnExplosion(_enemies[i].x + 4, _enemies[i].y + 4, Color565::Gold);
          _enemies[i].active = false;
          _score += 30;
        }
      }
    }

    // Move PowerUps
    for (int i = 0; i < 2; ++i) {
      if (_powerUps[i].active) {
        _powerUps[i].y += 45.0f * dt;
        if (_powerUps[i].y > SCREEN_HEIGHT) _powerUps[i].active = false;
        if (_shipX < _powerUps[i].x + 8 && _shipX + 12 > _powerUps[i].x &&
            110.0f < _powerUps[i].y + 8 && 124.0f > _powerUps[i].y) {
          if (_powerUps[i].type == 0) _spreadTimer = 7.0f;
          else if (_bombs < 5) _bombs++;
          _score += 25;
          _powerUps[i].active = false;
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
                spawnExplosion(_enemies[i].x + 4, _enemies[i].y + 4, (_enemies[i].type == 1) ? Color565::Magenta : Color565::Orange);
                if (random(0, 8) == 0) {
                  for (int p = 0; p < 2; ++p) {
                    if (!_powerUps[p].active) {
                      _powerUps[p].x = _enemies[i].x;
                      _powerUps[p].y = _enemies[i].y;
                      _powerUps[p].type = random(0, 2);
                      _powerUps[p].active = true;
                      break;
                    }
                  }
                }
                _enemies[i].active = false;
                _score += (_enemies[i].type == 1) ? 35 : 20;
              }
              break;
            }
          }
        }

        // Enemy vs Ship Collision (forgiving hitbox, 3 lives)
        if (_enemies[i].active && _invulnTimer <= 0.0f) {
          if (_shipX + 2.0f < _enemies[i].x + 8.0f && _shipX + 9.0f > _enemies[i].x + 1.0f &&
              112.0f < _enemies[i].y + 8.0f && 121.0f > _enemies[i].y + 1.0f) {
            _enemies[i].active = false;
            spawnExplosion(_shipX + 5, 116, Color565::Red);
            _lives--;
            if (_lives == 0) {
              triggerGameOver();
              return;
            }
            _invulnTimer = 1.6f;
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

    // Explosion Particles
    for (int i = 0; i < 12; ++i) {
      if (_parts[i].life > 0.0f) {
        gfx.fillRect((int16_t)_parts[i].x, (int16_t)_parts[i].y, 2, 2, _parts[i].color);
      }
    }

    // PowerUps
    for (int i = 0; i < 2; ++i) {
      if (_powerUps[i].active) {
        uint16_t pcol = (_powerUps[i].type == 0) ? Color565::Cyan : Color565::Gold;
        gfx.fillRoundRect((int16_t)_powerUps[i].x, (int16_t)_powerUps[i].y, 8, 8, 2, pcol);
        gfx.drawText((int16_t)_powerUps[i].x + 1, (int16_t)_powerUps[i].y, (_powerUps[i].type == 0) ? "S" : "B", Color565::Black, 1);
      }
    }

    // Player Ship (Sleek Fighter, blinks during invulnerability)
    if (_invulnTimer <= 0.0f || ((int)(_invulnTimer * 12) % 2 == 0)) {
      int16_t sx = (int16_t)_shipX;
      gfx.fillTriangle(sx + 5, 110, sx, 122, sx + 11, 122, Color565::SkyBlue);
      gfx.fillRect(sx + 4, 113, 3, 6, Color565::White);
      // Animated Engine Thrusters
      int16_t fl = ((millis() / 80) % 2 == 0) ? 4 : 2;
      gfx.fillRect(sx + 2, 123, 2, fl, Color565::Orange);
      gfx.fillRect(sx + 7, 123, 2, fl, Color565::Yellow);
    }

    drawHud(gfx);

    // Lives & Bomb Icons on HUD
    for (int i = 0; i < _lives; ++i) {
      gfx.fillCircle(42 + (i * 7), 6, 2, Color565::Red);
    }
    for (int i = 0; i < _bombs; ++i) {
      gfx.fillCircle(72 + (i * 7), 6, 2, Color565::Gold);
    }

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
