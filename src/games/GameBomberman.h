#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameBomberman : public IGame {
private:
  static constexpr int COLS = 11;
  static constexpr int ROWS = 9;
  static constexpr int CELL = 11;
  static constexpr int OFF_X = 3;
  static constexpr int OFF_Y = 20;

  // Tile types: 0 = Empty, 1 = Steel Wall, 2 = Brick Wall, 3 = Flame
  uint8_t _grid[ROWS][COLS];
  float   _flameTimer[ROWS][COLS];

  int8_t  _px, _py;
  int8_t  _pDir; // 0: Up, 1: Right, 2: Down, 3: Left
  float   _moveTimer;
  uint8_t _stage;

  struct Bomb {
    bool   active;
    int8_t x, y;
    float  timer;
    int8_t range;
  };
  Bomb _bombs[2];

  struct Enemy {
    bool   alive;
    int8_t x, y;
    int8_t dir;
    float  moveTimer;
  };
  Enemy _enemies[4];

  void setupStage() {
    for (int r = 0; r < ROWS; ++r) {
      for (int c = 0; c < COLS; ++c) {
        _flameTimer[r][c] = 0.0f;
        if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1 || (r % 2 == 0 && c % 2 == 0)) {
          _grid[r][c] = 1; // Indestructible Steel
        } else if ((r <= 2 && c <= 2)) {
          _grid[r][c] = 0; // Keep player spawn corner clear
        } else {
          _grid[r][c] = (random(0, 100) < 42) ? 2 : 0; // Destructible Brick
        }
      }
    }

    _px = 1;
    _py = 1;
    _pDir = 1; // Facing Right
    _moveTimer = 0.0f;

    for (int i = 0; i < 2; ++i) _bombs[i].active = false;

    uint8_t enemyCnt = min((int)(1 + _stage / 2), 3); // Fewer enemies (1 on stage 1)
    for (int i = 0; i < 4; ++i) {
      if (i < enemyCnt) {
        _enemies[i].alive = true;
        _enemies[i].x = COLS - 2 - (i % 2) * 2;
        _enemies[i].y = ROWS - 2 - (i / 2) * 2;
        _grid[_enemies[i].y][_enemies[i].x] = 0;
        if (_enemies[i].x - 1 > 1 && _grid[_enemies[i].y][_enemies[i].x - 1] == 2) {
          _grid[_enemies[i].y][_enemies[i].x - 1] = 0;
        }
        _enemies[i].dir = random(0, 4);
        _enemies[i].moveTimer = 0.0f;
      } else {
        _enemies[i].alive = false;
      }
    }
  }

  bool isWalkable(int8_t x, int8_t y) const {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return false;
    if (_grid[y][x] == 1 || _grid[y][x] == 2) return false;
    for (int i = 0; i < 2; ++i) {
      if (_bombs[i].active && _bombs[i].x == x && _bombs[i].y == y) return false;
    }
    return true;
  }

  void plantBomb() {
    for (int i = 0; i < 2; ++i) {
      if (_bombs[i].active && _bombs[i].x == _px && _bombs[i].y == _py) return;
    }
    for (int i = 0; i < 2; ++i) {
      if (!_bombs[i].active) {
        _bombs[i].active = true;
        _bombs[i].x = _px;
        _bombs[i].y = _py;
        _bombs[i].timer = 1.8f;
        _bombs[i].range = 3; // Longer 3-tile blast range!
        return;
      }
    }
  }

  void addFlame(int8_t x, int8_t y) {
    _grid[y][x] = 3;
    _flameTimer[y][x] = 0.45f;

    // Chain detonate other bombs
    for (int i = 0; i < 2; ++i) {
      if (_bombs[i].active && _bombs[i].x == x && _bombs[i].y == y && _bombs[i].timer > 0.05f) {
        _bombs[i].timer = 0.02f;
      }
    }

    // Check enemy hit
    for (int i = 0; i < 4; ++i) {
      if (_enemies[i].alive && _enemies[i].x == x && _enemies[i].y == y) {
        _enemies[i].alive = false;
        _score += 50;
      }
    }
  }

  void detonateBomb(int idx) {
    int8_t bx = _bombs[idx].x;
    int8_t by = _bombs[idx].y;
    int8_t rng = _bombs[idx].range;
    _bombs[idx].active = false;

    addFlame(bx, by);
    const int8_t dx[4] = { 0, 1, 0, -1 };
    const int8_t dy[4] = { -1, 0, 1, 0 };

    for (int d = 0; d < 4; ++d) {
      for (int step = 1; step <= rng; ++step) {
        int8_t nx = bx + dx[d] * step;
        int8_t ny = by + dy[d] * step;
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) break;
        if (_grid[ny][nx] == 1) break; // Steel stops flame
        if (_grid[ny][nx] == 2) {
          // Destroy brick and stop flame in this direction
          addFlame(nx, ny);
          _score += 10;
          break;
        }
        addFlame(nx, ny);
      }
    }
  }

public:
  const char* id() const override { return "bomber"; }
  const char* title() const override { return "Cyber Bomber"; }
  const char* desc() const override { return "A:Step B:Turn OK:Bomb"; }
  uint16_t themeColor() const override { return Color565::Orange; }

  void init() override {
    _score = 0;
    _stage = 1;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    setupStage();
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okShortReleased || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    const int8_t dx[4] = { 0, 1, 0, -1 };
    const int8_t dy[4] = { -1, 0, 1, 0 };

    // Controls:
    // Button B / Right = Rotate facing direction (90 deg clockwise, auto-picks walkable direction if tapped fast)
    if (input.rightPressed) {
      _pDir = (_pDir + 1) % 4;
    }

    // Button A / Left = Step forward in facing direction (or hold to walk)
    _moveTimer -= dt;
    if (input.leftPressed || (input.leftHeld && _moveTimer <= 0.0f)) {
      int8_t nx = _px + dx[_pDir];
      int8_t ny = _py + dy[_pDir];
      if (isWalkable(nx, ny)) {
        _px = nx;
        _py = ny;
        _moveTimer = 0.18f;
      } else if (input.leftPressed) {
        // Smart corner assist: if blocked straight ahead, auto-turn to an open adjacent tile!
        for (int turn = 1; turn <= 3; ++turn) {
          int8_t altDir = (_pDir + turn) % 4;
          if (isWalkable(_px + dx[altDir], _py + dy[altDir])) {
            _pDir = altDir;
            break;
          }
        }
      }
    }

    // Short-press BOOT (OK) = Plant Bomb!
    if (input.okShortReleased || input.bothPressed) {
      plantBomb();
    }

    // Update Bombs
    for (int i = 0; i < 2; ++i) {
      if (_bombs[i].active) {
        _bombs[i].timer -= dt;
        if (_bombs[i].timer <= 0.0f) {
          detonateBomb(i);
        }
      }
    }

    // Update Flames
    for (int r = 0; r < ROWS; ++r) {
      for (int c = 0; c < COLS; ++c) {
        if (_grid[r][c] == 3) {
          _flameTimer[r][c] -= dt;
          if (_flameTimer[r][c] <= 0.0f) {
            _grid[r][c] = 0;
          }
        }
      }
    }

    // Player has Flame Shield (immune to own bomb flames so you don't blow yourself up!)
    // Update Enemies (slower movement for easier hunting)
    bool anyAlive = false;
    float enemyInterval = max(0.38f, 0.65f - _stage * 0.04f);
    for (int i = 0; i < 4; ++i) {
      if (!_enemies[i].alive) continue;
      anyAlive = true;

      if (_grid[_enemies[i].y][_enemies[i].x] == 3) {
        _enemies[i].alive = false;
        _score += 50;
        continue;
      }

      _enemies[i].moveTimer += dt;
      if (_enemies[i].moveTimer >= enemyInterval) {
        _enemies[i].moveTimer = 0.0f;
        int8_t nx = _enemies[i].x + dx[_enemies[i].dir];
        int8_t ny = _enemies[i].y + dy[_enemies[i].dir];
        if (isWalkable(nx, ny) && random(0, 100) < 75) {
          _enemies[i].x = nx;
          _enemies[i].y = ny;
        } else {
          // Pick a new walkable direction
          int8_t startD = random(0, 4);
          for (int k = 0; k < 4; ++k) {
            int8_t d = (startD + k) % 4;
            if (isWalkable(_enemies[i].x + dx[d], _enemies[i].y + dy[d])) {
              _enemies[i].dir = d;
              _enemies[i].x += dx[d];
              _enemies[i].y += dy[d];
              break;
            }
          }
        }
      }

      if (_enemies[i].x == _px && _enemies[i].y == _py) {
        triggerGameOver();
        return;
      }
    }

    // Stage Clear!
    if (!anyAlive) {
      _score += 100;
      _stage++;
      setupStage();
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(12, 16, 26));

    // Draw Arena Grid
    for (int r = 0; r < ROWS; ++r) {
      for (int c = 0; c < COLS; ++c) {
        int16_t sx = OFF_X + c * CELL;
        int16_t sy = OFF_Y + r * CELL;
        uint8_t tile = _grid[r][c];

        if (tile == 1) {
          // Indestructible Steel Block
          gfx.fillRect(sx, sy, CELL - 1, CELL - 1, Color565::rgb(70, 82, 105));
          gfx.drawRect(sx, sy, CELL - 1, CELL - 1, Color565::GrayLight);
          gfx.drawPixel(sx + 2, sy + 2, Color565::White);
        } else if (tile == 2) {
          // Destructible Brick Wall
          gfx.fillRect(sx, sy, CELL - 1, CELL - 1, Color565::rgb(180, 75, 30));
          gfx.drawFastHLine(sx, sy + 4, CELL - 1, Color565::Gold);
          gfx.drawFastVLine(sx + 5, sy, 5, Color565::Gold);
        } else if (tile == 3) {
          // Explosion Flame
          gfx.fillRect(sx, sy, CELL - 1, CELL - 1, Color565::Orange);
          gfx.fillRect(sx + 2, sy + 2, CELL - 5, CELL - 5, Color565::Yellow);
        } else {
          // Floor Tile
          gfx.fillRect(sx, sy, CELL - 1, CELL - 1, Color565::rgb(20, 32, 24));
        }
      }
    }

    // Draw Bombs
    for (int i = 0; i < 2; ++i) {
      if (_bombs[i].active) {
        int16_t bx = OFF_X + _bombs[i].x * CELL + 5;
        int16_t by = OFF_Y + _bombs[i].y * CELL + 5;
        bool flash = ((int)(_bombs[i].timer * 8) % 2 == 0);
        gfx.fillCircle(bx, by, 4, flash ? Color565::Red : Color565::Black);
        gfx.drawCircle(bx, by, 4, Color565::White);
        gfx.drawPixel(bx + 2, by - 4, Color565::Yellow); // Fuse spark
      }
    }

    // Draw Enemies (Cyber Balloons)
    for (int i = 0; i < 4; ++i) {
      if (_enemies[i].alive) {
        int16_t ex = OFF_X + _enemies[i].x * CELL + 5;
        int16_t ey = OFF_Y + _enemies[i].y * CELL + 5;
        gfx.fillCircle(ex, ey, 4, Color565::Magenta);
        gfx.drawPixel(ex - 1, ey - 1, Color565::White);
        gfx.drawPixel(ex + 1, ey - 1, Color565::White);
      }
    }

    // Draw Player (Cyber Bomberman + Facing Direction Arrow)
    int16_t px = OFF_X + _px * CELL + 5;
    int16_t py = OFF_Y + _py * CELL + 5;
    gfx.fillCircle(px, py, 4, Color565::NeonCyan);
    gfx.drawCircle(px, py, 4, Color565::White);

    // Direction indicator dot outside player helmet so player knows which way A steps
    const int8_t dx[4] = { 0, 1, 0, -1 };
    const int8_t dy[4] = { -1, 0, 1, 0 };
    int16_t ax = px + dx[_pDir] * 5;
    int16_t ay = py + dy[_pDir] * 5;
    gfx.fillCircle(ax, ay, 1, Color565::Yellow);

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};
