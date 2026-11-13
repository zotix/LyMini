#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameDino : public IGame {
private:
  float _dinoY;
  float _dinoVY;
  bool _isGrounded;
  bool _isDucking;

  struct Obstacle {
    float x;
    uint8_t type; // 0: Small Cactus, 1: Double Cactus, 2: Pterodactyl Bird Low, 3: Bird High
    uint8_t w, h;
    bool active;
  };
  Obstacle _obstacles[3];

  float _gameSpeed;
  float _distance;
  bool _isNight;
  uint8_t _animFrame;
  float _animTimer;

  struct Dust {
    float x, y, vx, vy, life;
  };
  Dust _dust[4];

public:
  const char* id() const override { return "dino"; }
  const char* title() const override { return "Chrome Dino"; }
  const char* desc() const override { return "Jump & Duck obstacles!"; }
  uint16_t themeColor() const override { return Color565::Lime; }

  void spawnDust(float x, float y, int count) {
    for (int i = 0; i < 4 && count > 0; ++i) {
      if (_dust[i].life <= 0.0f) {
        _dust[i].x = x;
        _dust[i].y = y;
        _dust[i].vx = -random(20, 45);
        _dust[i].vy = -random(10, 30);
        _dust[i].life = 0.28f;
        count--;
      }
    }
  }

  void init() override {
    _dinoY = 96.0f;
    _dinoVY = 0.0f;
    _isGrounded = true;
    _isDucking = false;
    _gameSpeed = 72.0f; // Easier starting speed
    _distance = 0.0f;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    _isNight = false;
    _animFrame = 0;
    _animTimer = 0;

    for (int i = 0; i < 3; ++i) {
      _obstacles[i].active = false;
      _obstacles[i].x = 170.0f + i * 105.0f;
    }
    for (int i = 0; i < 4; ++i) _dust[i].life = 0.0f;
  }

  void spawnObstacle(int index, float minX) {
    _obstacles[index].x = minX + random(78, 138); // Wider spacing between obstacles
    uint8_t r = random(0, 10);
    if (_score > 250 && r >= 8) {
      // Pterodactyl Bird
      _obstacles[index].type = (r == 8) ? 2 : 3; // 2: low (duck), 3: high (jump)
      _obstacles[index].w = 11;
      _obstacles[index].h = 7;
    } else if (r >= 5) {
      // Double Cactus
      _obstacles[index].type = 1;
      _obstacles[index].w = 9;
      _obstacles[index].h = 14;
    } else {
      // Small Cactus
      _obstacles[index].type = 0;
      _obstacles[index].w = 6;
      _obstacles[index].h = 11;
    }
    _obstacles[index].active = true;
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Animation timer & running dust
    _animTimer += dt;
    if (_animTimer >= 0.1f) {
      _animTimer = 0.0f;
      _animFrame = (_animFrame + 1) % 2;
      if (_isGrounded && random(0, 3) == 0) spawnDust(19.0f, 108.0f, 1);
    }

    // Update dust particles
    for (int i = 0; i < 4; ++i) {
      if (_dust[i].life > 0.0f) {
        _dust[i].life -= dt;
        _dust[i].x += _dust[i].vx * dt;
        _dust[i].y += _dust[i].vy * dt;
      }
    }

    // Day/Night cycle every 500 score points (milestone-based, no flicker)
    _isNight = ((_score / 500) % 2 == 1);

    // Speed scales gently with distance
    _gameSpeed = 72.0f + sqrtf(_distance) * 1.5f;
    if (_gameSpeed > 155.0f) _gameSpeed = 155.0f;

    // Jump Input (Left Button or OK short release)
    if ((input.leftPressed || input.leftHeld || input.okShortReleased) && _isGrounded) {
      _dinoVY = -205.0f; // Higher, more forgiving jump impulse
      _isGrounded = false;
      spawnDust(22.0f, 108.0f, 3);
    }

    // Duck Input (Right Button)
    _isDucking = input.rightHeld && _isGrounded;
    if (input.rightHeld && !_isGrounded) {
      // Fast fall down
      _dinoVY += 420.0f * dt;
    }

    // Apply Gravity
    if (!_isGrounded) {
      _dinoVY += 460.0f * dt;
      _dinoY += _dinoVY * dt;

      if (_dinoY >= 96.0f) {
        _dinoY = 96.0f;
        _dinoVY = 0.0f;
        _isGrounded = true;
        spawnDust(22.0f, 108.0f, 3);
      }
    }

    // Move Obstacles
    float furthestX = 128.0f;
    for (int i = 0; i < 3; ++i) {
      if (_obstacles[i].active && _obstacles[i].x > furthestX) {
        furthestX = _obstacles[i].x;
      }
    }

    for (int i = 0; i < 3; ++i) {
      if (!_obstacles[i].active) {
        spawnObstacle(i, furthestX);
        furthestX = _obstacles[i].x;
      } else {
        _obstacles[i].x -= _gameSpeed * dt;
        if (_obstacles[i].x < -20.0f) {
          spawnObstacle(i, furthestX);
          furthestX = _obstacles[i].x;
        }
      }
    }

    // Forgiving Collision Detection (shrunk hitbox so grazing edges is safe)
    int16_t dinoBoxX = 20;
    int16_t dinoBoxY = _isDucking ? (_dinoY + 7) : (_dinoY + 2);
    int16_t dinoBoxW = _isDucking ? 12 : 7;
    int16_t dinoBoxH = _isDucking ? 6 : 10;

    for (int i = 0; i < 3; ++i) {
      if (!_obstacles[i].active) continue;

      int16_t obsX = (int16_t)_obstacles[i].x;
      int16_t obsY = 110 - _obstacles[i].h; // Ground baseline is 110
      if (_obstacles[i].type == 2) obsY = 92; // Low flying bird
      if (_obstacles[i].type == 3) obsY = 80; // High flying bird

      // AABB Box overlap check
      if (dinoBoxX < obsX + _obstacles[i].w &&
          dinoBoxX + dinoBoxW > obsX &&
          dinoBoxY < obsY + _obstacles[i].h &&
          dinoBoxY + dinoBoxH > obsY) {
        triggerGameOver();
        return;
      }
    }

    _distance += _gameSpeed * dt;
    _score = (uint32_t)(_distance * 0.1f);
  }

  void render(DisplayCanvas& gfx) override {
    uint16_t groundColor = _isNight ? Color565::GrayMid : Color565::Gold;
    uint16_t dinoColor = _isNight ? Color565::Lime : Color565::rgb(40, 180, 50);

    // Gradient Sky Background
    for (int16_t y = 12; y < 110; y += 4) {
      uint8_t t = (y - 12) * 255 / 98;
      uint16_t col = _isNight
        ? Color565::rgb(6 + t / 25, 10 + t / 18, 28 + t / 10)
        : Color565::rgb(20 + t / 4, 150 + t / 4, 235);
      gfx.fillRect(0, y, SCREEN_WIDTH, 4, col);
    }
    gfx.fillRect(0, 110, SCREEN_WIDTH, SCREEN_HEIGHT - 110, _isNight ? Color565::rgb(22, 24, 34) : Color565::rgb(175, 130, 65));

    // Stars / Sun / Moon & Clouds
    if (_isNight) {
      gfx.drawPixel(20, 25, Color565::White);
      gfx.drawPixel(45, 34, Color565::Cyan);
      gfx.drawPixel(60, 20, Color565::Yellow);
      gfx.drawPixel(85, 38, Color565::White);
      gfx.fillCircle(104, 24, 6, Color565::White);
      gfx.fillCircle(101, 23, 5, Color565::rgb(8, 12, 30)); // Crescent moon
    } else {
      gfx.fillCircle(108, 24, 6, Color565::Yellow);
      int16_t cx1 = 128 - (((int)(_distance * 0.2f)) % 150);
      gfx.fillRoundRect(cx1, 26, 22, 7, 3, Color565::White);
      gfx.fillCircle(cx1 + 8, 25, 4, Color565::White);
      int16_t cx2 = 128 - (((int)(_distance * 0.2f + 75)) % 150);
      gfx.fillRoundRect(cx2, 42, 18, 6, 3, Color565::White);
    }

    // Ground line & terrain dashes
    gfx.drawFastHLine(0, 110, SCREEN_WIDTH, groundColor);
    int16_t dashOffset = -((int)_distance % 12);
    for (int16_t x = dashOffset; x < SCREEN_WIDTH; x += 16) {
      gfx.drawFastHLine(x, 114, 6, groundColor);
    }

    // Dust particles
    for (int i = 0; i < 4; ++i) {
      if (_dust[i].life > 0.0f) {
        gfx.fillRect((int16_t)_dust[i].x, (int16_t)_dust[i].y, 2, 2, Color565::White);
      }
    }

    // Render Dino
    int16_t dx = 18;
    int16_t dy = (int16_t)_dinoY;

    if (_isDucking) {
      gfx.fillRect(dx, dy + 6, 16, 8, dinoColor);
      gfx.fillRect(dx + 12, dy + 4, 5, 5, dinoColor); // Head
      gfx.drawPixel(dx + 15, dy + 5, Color565::Black); // Eye
      // Ducking legs
      if (_animFrame == 0) {
        gfx.drawFastHLine(dx + 2, dy + 14, 4, dinoColor);
      } else {
        gfx.drawFastHLine(dx + 8, dy + 14, 4, dinoColor);
      }
    } else {
      // Body
      gfx.fillRect(dx, dy, 10, 12, dinoColor);
      // Head
      gfx.fillRect(dx + 6, dy - 4, 8, 8, dinoColor);
      gfx.drawPixel(dx + 11, dy - 2, Color565::Black); // Eye
      // Snout
      gfx.fillRect(dx + 12, dy, 3, 4, dinoColor);
      // Legs
      if (_isGrounded) {
        if (_animFrame == 0) {
          gfx.drawFastVLine(dx + 2, dy + 12, 3, dinoColor);
        } else {
          gfx.drawFastVLine(dx + 7, dy + 12, 3, dinoColor);
        }
      } else {
        gfx.drawFastVLine(dx + 4, dy + 12, 2, dinoColor);
      }
    }

    // Render Obstacles
    for (int i = 0; i < 3; ++i) {
      if (!_obstacles[i].active) continue;
      int16_t ox = (int16_t)_obstacles[i].x;

      if (_obstacles[i].type == 0) {
        // Small Cactus
        int16_t oy = 110 - 12;
        gfx.fillRect(ox + 2, oy, 2, 12, Color565::Green);
        gfx.fillRect(ox, oy + 3, 2, 4, Color565::Green);
        gfx.fillRect(ox + 4, oy + 2, 2, 5, Color565::Green);
      } else if (_obstacles[i].type == 1) {
        // Double Cactus
        int16_t oy = 110 - 16;
        gfx.fillRect(ox + 2, oy, 3, 16, Color565::Green);
        gfx.fillRect(ox + 7, oy + 3, 3, 13, Color565::Green);
      } else {
        // Pterodactyl Bird
        int16_t oy = (_obstacles[i].type == 2) ? 92 : 80;
        gfx.fillRect(ox + 3, oy + 2, 6, 4, Color565::Crimson); // Body
        gfx.fillRect(ox, oy + 1, 3, 3, Color565::Crimson);     // Head / Beak
        // Wings flapping
        if (_animFrame == 0) {
          gfx.fillRect(ox + 4, oy - 2, 4, 3, Color565::Coral); // Wing up
        } else {
          gfx.fillRect(ox + 4, oy + 6, 4, 3, Color565::Coral); // Wing down
        }
      }
    }

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
