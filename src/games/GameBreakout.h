#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameBreakout : public IGame {
private:
  static const uint8_t BRICK_ROWS = 5;
  static const uint8_t BRICK_COLS = 8;
  uint8_t _bricks[BRICK_ROWS][BRICK_COLS]; // 0: destroyed, 1: normal, 2: iron (needs 2 hits)

  float _paddleX;
  float _paddleW;
  float _ballX, _ballY;
  float _ballVX, _ballVY;
  bool _ballStuck;
  uint8_t _lives;

  struct Capsule {
    float x, y;
    uint8_t type; // 0: Wide Paddle, 1: Slow Ball, 2: Extra Life
    bool active;
  };
  Capsule _capsules[2];

  static const uint16_t ROW_COLORS[5];

public:
  const char* id() const override { return "breakout"; }
  const char* title() const override { return "Neon Breakout"; }
  const char* desc() const override { return "Smash all rainbow bricks!"; }
  uint16_t themeColor() const override { return Color565::Gold; }

  void init() override {
    _paddleW = 26.0f;
    _paddleX = (SCREEN_WIDTH - _paddleW) / 2.0f;
    _lives = 3;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    for (int i = 0; i < 2; ++i) _capsules[i].active = false;

    // Reset Bricks
    for (int r = 0; r < BRICK_ROWS; ++r) {
      for (int c = 0; c < BRICK_COLS; ++c) {
        _bricks[r][c] = (r == 0 && (c == 2 || c == 5)) ? 2 : 1;
      }
    }

    resetBall();
  }

  void resetBall() {
    _ballStuck = true;
    _ballX = _paddleX + (_paddleW / 2.0f);
    _ballY = 117.0f;
    _ballVX = 55.0f;
    _ballVY = -75.0f;
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Paddle Movement
    float pSpeed = 115.0f;
    if (input.leftHeld) _paddleX -= pSpeed * dt;
    if (input.rightHeld) _paddleX += pSpeed * dt;
    if (_paddleX < 2.0f) _paddleX = 2.0f;
    if (_paddleX > SCREEN_WIDTH - _paddleW - 2.0f) _paddleX = SCREEN_WIDTH - _paddleW - 2.0f;

    // Launch Ball
    if (_ballStuck) {
      _ballX = _paddleX + (_paddleW / 2.0f);
      _ballY = 117.0f;
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        _ballStuck = false;
      }
      return;
    }

    // Move Ball
    _ballX += _ballVX * dt;
    _ballY += _ballVY * dt;

    // Bounce off side walls
    if (_ballX <= 3.0f) {
      _ballX = 3.0f;
      _ballVX = -_ballVX;
    } else if (_ballX >= SCREEN_WIDTH - 6.0f) {
      _ballX = SCREEN_WIDTH - 6.0f;
      _ballVX = -_ballVX;
    }

    // Bounce off ceiling
    if (_ballY <= 15.0f) {
      _ballY = 15.0f;
      _ballVY = -_ballVY;
    }

    // Bounce off Paddle
    if (_ballVY > 0 && _ballY >= 118.0f && _ballY <= 123.0f) {
      if (_ballX + 3.0f >= _paddleX && _ballX <= _paddleX + _paddleW) {
        float offset = (_ballX + 1.5f) - (_paddleX + (_paddleW / 2.0f));
        _ballVY = -fabs(_ballVY);
        _ballVX = offset * 6.0f;
      }
    }

    // Collision with Bricks
    // Brick dimensions: X offset 4, Y offset 20. Each brick is 14 px wide, 6 px high + 1px spacing
    bool anyBricksLeft = false;
    for (int r = 0; r < BRICK_ROWS; ++r) {
      for (int c = 0; c < BRICK_COLS; ++c) {
        if (_bricks[r][c] > 0) {
          anyBricksLeft = true;
          int16_t bx = 4 + c * 15;
          int16_t by = 20 + r * 8;

          if (_ballX + 3.0f >= bx && _ballX <= bx + 14 &&
              _ballY + 3.0f >= by && _ballY <= by + 7) {
            _bricks[r][c]--;
            _score += 15;
            _ballVY = -_ballVY;

            // Chance to drop power-up capsule
            if (random(0, 10) == 0) {
              for (int k = 0; k < 2; ++k) {
                if (!_capsules[k].active) {
                  _capsules[k].x = bx + 4;
                  _capsules[k].y = by;
                  _capsules[k].type = random(0, 3);
                  _capsules[k].active = true;
                  break;
                }
              }
            }
            goto collision_done;
          }
        }
      }
    }
collision_done:

    if (!anyBricksLeft) {
      _score += 200;
      init(); // Clear stage and replay!
      return;
    }

    // Move Capsules
    for (int i = 0; i < 2; ++i) {
      if (_capsules[i].active) {
        _capsules[i].y += 45.0f * dt;
        if (_capsules[i].y > SCREEN_HEIGHT) _capsules[i].active = false;

        // Catch capsule
        if (_capsules[i].x >= _paddleX && _capsules[i].x <= _paddleX + _paddleW &&
            _capsules[i].y >= 118.0f && _capsules[i].y <= 124.0f) {
          _capsules[i].active = false;
          _score += 30;
          if (_capsules[i].type == 0) _paddleW = 36.0f; // Wide Paddle
          else if (_capsules[i].type == 1) { // Slow ball
            _ballVX *= 0.8f;
            _ballVY *= 0.8f;
          } else if (_capsules[i].type == 2) {
            _lives++;
          }
        }
      }
    }

    // Ball fell below bottom
    if (_ballY > SCREEN_HEIGHT) {
      _lives--;
      if (_lives == 0) {
        triggerGameOver();
      } else {
        _paddleW = 26.0f;
        resetBall();
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Dark Backdrop
    gfx.fillRect(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, Color565::rgb(15, 15, 25));

    // Outer border
    gfx.drawFastVLine(0, 12, SCREEN_HEIGHT - 12, Color565::Navy);
    gfx.drawFastVLine(SCREEN_WIDTH - 1, 12, SCREEN_HEIGHT - 12, Color565::Navy);

    // Bricks
    for (int r = 0; r < BRICK_ROWS; ++r) {
      for (int c = 0; c < BRICK_COLS; ++c) {
        if (_bricks[r][c] > 0) {
          int16_t bx = 4 + c * 15;
          int16_t by = 20 + r * 8;
          uint16_t col = (_bricks[r][c] == 2) ? Color565::GrayLight : ROW_COLORS[r];
          gfx.fillRect(bx, by, 14, 6, col);
          gfx.drawPixel(bx, by, Color565::White); // Shading highlight
        }
      }
    }

    // Power-up Capsules
    for (int i = 0; i < 2; ++i) {
      if (_capsules[i].active) {
        uint16_t ccol = (_capsules[i].type == 0) ? Color565::Cyan : (_capsules[i].type == 1 ? Color565::Lime : Color565::PinkHot);
        gfx.fillRoundRect((int16_t)_capsules[i].x, (int16_t)_capsules[i].y, 8, 4, 2, ccol);
      }
    }

    // Ball
    gfx.fillRect((int16_t)_ballX, (int16_t)_ballY, 3, 3, Color565::White);

    // Paddle
    gfx.fillRoundRect((int16_t)_paddleX, 120, (int16_t)_paddleW, 4, 2, Color565::Gold);

    drawHud(gfx);

    // Lives count indicator on HUD
    for (int i = 0; i < _lives; ++i) {
      gfx.fillCircle(SCREEN_WIDTH / 2 - 12 + (i * 8), 6, 2, Color565::Red);
    }

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};

const uint16_t GameBreakout::ROW_COLORS[5] = {
  Color565::Red,
  Color565::Orange,
  Color565::Yellow,
  Color565::Lime,
  Color565::Cyan
};
