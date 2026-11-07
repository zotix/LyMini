#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GamePong : public IGame {
private:
  float _ballX, _ballY;
  float _ballVX, _ballVY;
  float _playerX;
  float _aiX;
  uint8_t _playerScore;
  uint8_t _aiScore;
  float _trailX[4];
  float _trailY[4];
  uint8_t _rallyCount;

public:
  const char* id() const override { return "pong"; }
  const char* title() const override { return "Cyber Pong"; }
  const char* desc() const override { return "Left/Right vs AI!"; }
  uint16_t themeColor() const override { return Color565::Cyan; }

  void init() override {
    _playerScore = 0;
    _aiScore = 0;
    _score = 0;
    _rallyCount = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    _playerX = 64.0f - 16.0f;
    _aiX = 64.0f - 11.0f;
    resetBall(1);
  }

  void resetBall(int dir) {
    _ballX = 64.0f;
    _ballY = 64.0f;
    _ballVX = (random(0, 2) == 0 ? -36.0f : 36.0f);
    _ballVY = dir * 50.0f;
    for (int i = 0; i < 4; ++i) {
      _trailX[i] = _ballX;
      _trailY[i] = _ballY;
    }
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okShortReleased || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Player Paddle Movement (Wider 32px paddle & faster response)
    float pSpeed = 128.0f;
    if (input.leftHeld) _playerX -= pSpeed * dt;
    if (input.rightHeld) _playerX += pSpeed * dt;
    if (_playerX < 2.0f) _playerX = 2.0f;
    if (_playerX > SCREEN_WIDTH - 34.0f) _playerX = SCREEN_WIDTH - 34.0f;

    // AI Paddle Tracking (Slower so player can easily score)
    float aiCenter = _aiX + 11.0f;
    float targetX = _ballX;
    float aiSpeed = 44.0f + (_aiScore * 3.0f);
    if (targetX < aiCenter - 3.0f) _aiX -= aiSpeed * dt;
    else if (targetX > aiCenter + 3.0f) _aiX += aiSpeed * dt;
    if (_aiX < 2.0f) _aiX = 2.0f;
    if (_aiX > SCREEN_WIDTH - 24.0f) _aiX = SCREEN_WIDTH - 24.0f;

    // Shift ball trails
    for (int i = 3; i > 0; --i) {
      _trailX[i] = _trailX[i - 1];
      _trailY[i] = _trailY[i - 1];
    }
    _trailX[0] = _ballX;
    _trailY[0] = _ballY;

    // Move Ball
    _ballX += _ballVX * dt;
    _ballY += _ballVY * dt;

    // Bounce off left/right side walls
    if (_ballX <= 3.0f) {
      _ballX = 3.0f;
      _ballVX = -_ballVX;
    } else if (_ballX >= SCREEN_WIDTH - 6.0f) {
      _ballX = SCREEN_WIDTH - 6.0f;
      _ballVX = -_ballVX;
    }

    // Collision with Player Paddle (Bottom: Y = 118, Width = 32)
    if (_ballVY > 0 && _ballY >= 114.0f && _ballY <= 122.0f) {
      if (_ballX + 4.0f >= _playerX && _ballX <= _playerX + 32.0f) {
        float hitOffset = (_ballX + 2.0f) - (_playerX + 16.0f);
        _ballVY = -(_ballVY * 1.03f);
        if (fabs(_ballVY) > 130.0f) _ballVY = -130.0f;
        _ballVX = hitOffset * 4.8f;

        _rallyCount++;
        _score += 15 + (_rallyCount * 2);
      }
    }

    // Collision with AI Paddle (Top: Y = 20, Width = 22)
    if (_ballVY < 0 && _ballY <= 24.0f && _ballY >= 18.0f) {
      if (_ballX + 4.0f >= _aiX && _ballX <= _aiX + 22.0f) {
        float hitOffset = (_ballX + 2.0f) - (_aiX + 11.0f);
        _ballVY = -(_ballVY * 1.03f);
        if (fabs(_ballVY) > 130.0f) _ballVY = 130.0f;
        _ballVX = hitOffset * 5.0f;
      }
    }

    // Missed - AI scores
    if (_ballY > SCREEN_HEIGHT) {
      _aiScore++;
      if (_aiScore >= 5) {
        triggerGameOver();
      } else {
        resetBall(-1);
      }
    }

    // Scored on AI
    if (_ballY < 14.0f) {
      _playerScore++;
      _score += 50;
      if (_playerScore >= 5) {
        _score += 200;
        triggerGameOver();
      } else {
        resetBall(1);
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Dark Space Court
    gfx.fillRect(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, Color565::rgb(10, 15, 30));

    // Court Net Dashed Line
    for (int x = 4; x < SCREEN_WIDTH; x += 8) {
      gfx.drawFastHLine(x, 68, 4, Color565::Navy);
    }

    // Court Side Borders
    gfx.drawFastVLine(0, 12, SCREEN_HEIGHT - 12, Color565::NeonCyan);
    gfx.drawFastVLine(SCREEN_WIDTH - 1, 12, SCREEN_HEIGHT - 12, Color565::NeonCyan);

    // AI Score & Player Score in background
    char sbuf[8];
    snprintf(sbuf, sizeof(sbuf), "%d", _aiScore);
    gfx.drawText(6, 30, sbuf, Color565::GrayDark, 2);
    snprintf(sbuf, sizeof(sbuf), "%d", _playerScore);
    gfx.drawText(6, 85, sbuf, Color565::GrayDark, 2);

    // Ball Trails (Glow effect)
    for (int i = 3; i > 0; --i) {
      gfx.fillRect((int16_t)_trailX[i], (int16_t)_trailY[i], 3, 3, Color565::rgb(20 * (4 - i), 50 * (4 - i), 60 * (4 - i)));
    }

    // Ball
    gfx.fillRect((int16_t)_ballX, (int16_t)_ballY, 4, 4, Color565::White);

    // AI Paddle (Magenta, 22px)
    gfx.fillRoundRect((int16_t)_aiX, 20, 22, 4, 2, Color565::Magenta);

    // Player Paddle (Cyan, wider 32px)
    gfx.fillRoundRect((int16_t)_playerX, 118, 32, 4, 2, Color565::Cyan);

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
