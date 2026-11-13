#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameSnake : public IGame {
private:
  struct Point {
    int8_t x, y;
  };
  Point _body[64];
  uint8_t _length;
  int8_t _dir; // 0: Up, 1: Right, 2: Down, 3: Left
  Point _food;
  Point _bonusFood;
  bool _bonusActive;
  float _bonusTimer;
  float _stepTimer;
  float _stepDelay;
  float _tongueTimer;

public:
  const char* id() const override { return "snake"; }
  const char* title() const override { return "Retro Snake"; }
  const char* desc() const override { return "2-Button Relative Turn!"; }
  uint16_t themeColor() const override { return Color565::Emerald; }

  void init() override {
    _length = 4;
    _dir = 1; // Moving Right
    _stepTimer = 0.0f;
    _stepDelay = 0.22f; // Slower, easier step speed
    _bonusActive = false;
    _bonusTimer = 0.0f;
    _tongueTimer = 0.0f;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    // Grid size: 16x14 cells (cell size 7x7 px), Y offset 16
    for (int i = 0; i < _length; ++i) {
      _body[i].x = 6 - i;
      _body[i].y = 7;
    }
    spawnFood();
  }

  void spawnFood() {
    bool onBody;
    do {
      onBody = false;
      _food.x = random(1, 16);
      _food.y = random(1, 14);
      for (int i = 0; i < _length; ++i) {
        if (_body[i].x == _food.x && _body[i].y == _food.y) {
          onBody = true;
          break;
        }
      }
    } while (onBody);
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Relative 2-Button steering:
    // Left button = Turn Left relative to snake head (-1 mod 4)
    // Right button = Turn Right relative to snake head (+1 mod 4)
    if (input.leftPressed) {
      _dir = (_dir + 3) % 4;
    } else if (input.rightPressed) {
      _dir = (_dir + 1) % 4;
    }

    _tongueTimer += dt;
    if (_bonusActive) {
      _bonusTimer -= dt;
      if (_bonusTimer <= 0.0f) _bonusActive = false;
    }

    // Normal relaxed pace (BOOT hold sprint disabled so holding BOOT to exit is safe)
    float currentDelay = _stepDelay;

    _stepTimer += dt;
    if (_stepTimer >= currentDelay) {
      _stepTimer = 0.0f;

      // Calculate next head position
      Point nextHead = _body[0];
      if (_dir == 0) nextHead.y--;
      else if (_dir == 1) nextHead.x++;
      else if (_dir == 2) nextHead.y++;
      else if (_dir == 3) nextHead.x--;

      // Wrap-around walls so hitting the border doesn't kill the player!
      if (nextHead.x < 1)  nextHead.x = 16;
      if (nextHead.x > 16) nextHead.x = 1;
      if (nextHead.y < 1)  nextHead.y = 14;
      if (nextHead.y > 14) nextHead.y = 1;

      // Self collision
      for (int i = 0; i < _length; ++i) {
        if (_body[i].x == nextHead.x && _body[i].y == nextHead.y) {
          triggerGameOver();
          return;
        }
      }

      // Shift body
      for (int i = _length; i > 0; --i) {
        _body[i] = _body[i - 1];
      }
      _body[0] = nextHead;

      // Eat Food
      if (nextHead.x == _food.x && nextHead.y == _food.y) {
        _score += 15;
        if (_length < 63) _length++;
        if (_stepDelay > 0.12f) _stepDelay -= 0.002f;
        spawnFood();
        if (!_bonusActive && random(0, 4) == 0) {
          _bonusFood.x = random(1, 16);
          _bonusFood.y = random(1, 14);
          _bonusActive = true;
          _bonusTimer = 7.0f;
        }
      }
      if (_bonusActive && nextHead.x == _bonusFood.x && nextHead.y == _bonusFood.y) {
        _score += 50;
        _bonusActive = false;
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Checkerboard Garden Lawn
    for (int y = 0; y < 15; ++y) {
      for (int x = 0; x < 17; ++x) {
        uint16_t c = ((x + y) % 2 == 0) ? Color565::rgb(15, 45, 20) : Color565::rgb(18, 52, 23);
        gfx.fillRect(x * 7 + 4, y * 7 + 16, 7, 7, c);
      }
    }

    // Outer Boundary Wall
    gfx.drawRect(4, 16, 17 * 7, 15 * 7, Color565::Gold);

    // Food (Crisp Red Apple with green leaf & highlight)
    int16_t fx = _food.x * 7 + 4;
    int16_t fy = _food.y * 7 + 16;
    gfx.fillCircle(fx + 3, fy + 4, 3, Color565::Red);
    gfx.drawPixel(fx + 2, fy + 3, Color565::White);
    gfx.drawPixel(fx + 3, fy + 1, Color565::Lime); // Leaf

    // Golden Apple Bonus
    if (_bonusActive && ((int)(_bonusTimer * 6) % 2 == 0 || _bonusTimer > 2.0f)) {
      int16_t gx = _bonusFood.x * 7 + 4;
      int16_t gy = _bonusFood.y * 7 + 16;
      gfx.fillCircle(gx + 3, gy + 4, 3, Color565::Gold);
      gfx.drawPixel(gx + 2, gy + 3, Color565::White);
    }

    // Snake Body
    for (int i = _length - 1; i >= 0; --i) {
      int16_t bx = _body[i].x * 7 + 4;
      int16_t by = _body[i].y * 7 + 16;
      if (i == 0) {
        // Head (Bright Emerald Green with eyes + flicking tongue)
        gfx.fillRoundRect(bx + 1, by + 1, 6, 6, 2, Color565::Lime);
        if (((int)(_tongueTimer * 4) % 4) == 0) {
          int8_t dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
          gfx.drawPixel(bx + 3 + dx[_dir] * 4, by + 3 + dy[_dir] * 4, Color565::Red);
        }
        // Eyes
        if (_dir == 0 || _dir == 2) {
          gfx.drawPixel(bx + 2, by + 3, Color565::Black);
          gfx.drawPixel(bx + 5, by + 3, Color565::Black);
        } else {
          gfx.drawPixel(bx + 3, by + 2, Color565::Black);
          gfx.drawPixel(bx + 3, by + 5, Color565::Black);
        }
      } else {
        // Body segments (Smooth Gradient)
        uint8_t g = max(90, 220 - i * 3);
        uint16_t segColor = Color565::rgb(15, g, 90);
        gfx.fillRect(bx + 1, by + 1, 5, 5, segColor);
      }
    }

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
