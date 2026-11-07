#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameTimberman : public IGame {
private:
  // Tree branches: 0: None, 1: Left Branch, 2: Right Branch
  uint8_t _branches[6];
  uint8_t _playerSide; // 0: Left, 1: Right
  float _timeRemaining;
  float _maxTime;
  uint8_t _chopAnim; // 0: Idle, 1: Swing
  float _chopAnimTimer;

  struct WoodChip {
    float x, y;
    float vx, vy;
    bool active;
  };
  WoodChip _chips[4];

public:
  const char* id() const override { return "timber"; }
  const char* title() const override { return "Timberman"; }
  const char* desc() const override { return "Chop left & right fast!"; }
  uint16_t themeColor() const override { return Color565::Coral; }

  void init() override {
    _playerSide = 0; // Starts on left
    _maxTime = 7.5f; // Generous starting time
    _timeRemaining = _maxTime;
    _score = 0;
    _chopAnim = 0;
    _chopAnimTimer = 0.0f;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    for (int i = 0; i < 6; ++i) {
      if (i < 3) _branches[i] = 0; // First 3 trunk pieces safe
      else addBranch(i);
    }

    for (int i = 0; i < 4; ++i) _chips[i].active = false;
  }

  void addBranch(int index) {
    uint8_t prev  = (index > 0) ? _branches[index - 1] : 0;
    uint8_t prev2 = (index > 1) ? _branches[index - 2] : 0;
    if (prev != 0 || (prev2 != 0 && random(0, 2) == 0)) {
      _branches[index] = 0; // Space branches out for easier reaction
    } else {
      uint8_t r = random(0, 3);
      _branches[index] = r; // 0: none, 1: left, 2: right
    }
  }

  void chop(uint8_t side) {
    _playerSide = side;
    _chopAnim = 1;
    _chopAnimTimer = 0.08f;

    // Spawn flying wood chips
    for (int i = 0; i < 4; ++i) {
      _chips[i].x = 64.0f + (side == 0 ? -10.0f : 10.0f);
      _chips[i].y = 96.0f;
      _chips[i].vx = (side == 0 ? 1.0f : -1.0f) * random(50, 110);
      _chips[i].vy = -random(30, 80);
      _chips[i].active = true;
    }

    // Check if player chopped directly into a branch on their side
    if (_branches[0] == (_playerSide + 1)) {
      triggerGameOver();
      return;
    }

    // Shift tree down
    for (int i = 0; i < 5; ++i) {
      _branches[i] = _branches[i + 1];
    }
    addBranch(5);

    // Check if new lower branch squashes player
    if (_branches[0] == (_playerSide + 1)) {
      triggerGameOver();
      return;
    }

    _score++;
    // Add generous time back
    float timeGain = max(0.32f, 0.65f - (_score * 0.002f));
    _timeRemaining = min(_maxTime, _timeRemaining + timeGain);
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Chop anim timer
    if (_chopAnim > 0) {
      _chopAnimTimer -= dt;
      if (_chopAnimTimer <= 0.0f) _chopAnim = 0;
    }

    // Wood chips physics
    for (int i = 0; i < 4; ++i) {
      if (_chips[i].active) {
        _chips[i].x += _chips[i].vx * dt;
        _chips[i].y += _chips[i].vy * dt;
        _chips[i].vy += 220.0f * dt;
        if (_chips[i].y > SCREEN_HEIGHT) _chips[i].active = false;
      }
    }

    // Controls: Left chops left, Right chops right, OK short release chops current side
    if (input.leftPressed) {
      chop(0);
    } else if (input.rightPressed) {
      chop(1);
    } else if (input.okShortReleased) {
      chop(_playerSide);
    }

    // Drain timer slowly so player has plenty of time to think
    _timeRemaining -= dt * 0.62f;
    if (_timeRemaining <= 0.0f) {
      _timeRemaining = 0.0f;
      triggerGameOver();
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Sunny Forest Sky
    gfx.fillRect(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, Color565::SkyBlue);

    // Mountains in background
    gfx.fillTriangle(0, 100, 35, 60, 70, 100, Color565::rgb(60, 100, 140));
    gfx.fillTriangle(50, 100, 90, 55, 128, 100, Color565::rgb(50, 90, 130));

    // Green Grass Ground
    gfx.fillRect(0, 108, SCREEN_WIDTH, 20, Color565::Green);
    gfx.drawFastHLine(0, 108, SCREEN_WIDTH, Color565::Emerald);

    // Tree Trunk (Center: X = 52, W = 24)
    gfx.fillRect(52, 12, 24, 96, Color565::rgb(140, 80, 40));
    gfx.fillRect(54, 12, 4, 96, Color565::rgb(170, 100, 50)); // Bark highlight
    gfx.fillRect(70, 12, 4, 96, Color565::rgb(110, 60, 30));  // Bark shadow

    // Tree Branches
    for (int i = 0; i < 6; ++i) {
      int16_t by = 92 - (i * 16);
      if (_branches[i] == 1) {
        // Left Branch
        gfx.fillRect(24, by, 28, 8, Color565::rgb(120, 70, 35));
        gfx.fillCircle(24, by + 4, 6, Color565::rgb(30, 120, 40)); // Pine leaves
      } else if (_branches[i] == 2) {
        // Right Branch
        gfx.fillRect(76, by, 28, 8, Color565::rgb(120, 70, 35));
        gfx.fillCircle(104, by + 4, 6, Color565::rgb(30, 120, 40));
      }
    }

    // Flying wood chips
    for (int i = 0; i < 4; ++i) {
      if (_chips[i].active) {
        gfx.fillRect((int16_t)_chips[i].x, (int16_t)_chips[i].y, 3, 3, Color565::Gold);
      }
    }

    // Lumberjack Character
    int16_t px = (_playerSide == 0) ? 32 : 82;
    int16_t py = 92;

    // Red Plaid Shirt & Blue Jeans
    gfx.fillRect(px + 3, py + 5, 8, 8, Color565::Red);
    gfx.fillRect(px + 4, py + 13, 6, 6, Color565::Blue);
    // Face & Brown Beanie
    gfx.fillRect(px + 4, py, 6, 5, Color565::rgb(240, 180, 140));
    gfx.fillRect(px + 3, py - 2, 8, 3, Color565::rgb(140, 70, 20));

    // Axe
    if (_chopAnim == 1) {
      int16_t ax = (_playerSide == 0) ? (px + 10) : (px - 6);
      gfx.drawFastHLine(ax, py + 8, 8, Color565::rgb(160, 100, 50));
      gfx.fillRect((_playerSide == 0 ? ax + 6 : ax), py + 6, 3, 5, Color565::White);
    }

    // Time Bar at top of playfield
    float pct = _timeRemaining / _maxTime;
    int16_t barW = (int16_t)(100.0f * pct);
    uint16_t barCol = (pct > 0.5f) ? Color565::Lime : ((pct > 0.25f) ? Color565::Gold : Color565::Red);
    gfx.drawRect(13, 16, 102, 6, Color565::White);
    gfx.fillRect(14, 17, barW, 4, barCol);

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
