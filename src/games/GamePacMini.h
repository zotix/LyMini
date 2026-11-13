#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GamePacMini : public IGame {
private:
  // 1=wall, 0=dot, 2=empty, 3=power pellet
  uint8_t _maze[11][13];
  int8_t _px, _py, _pdir;
  int8_t _gx[2], _gy[2];
  float _moveTimer, _ghostTimer, _frightTimer, _animT;
  uint8_t _dotsLeft;

  void resetMaze() {
    const uint8_t tmpl[11][13] = {
      {1,1,1,1,1,1,1,1,1,1,1,1,1},
      {1,3,0,0,0,0,1,0,0,0,0,3,1},
      {1,0,1,1,0,0,0,0,0,1,1,0,1},
      {1,0,0,0,0,1,0,1,0,0,0,0,1},
      {1,0,1,0,0,0,0,0,0,0,1,0,1},
      {1,0,0,0,1,0,2,0,1,0,0,0,1},
      {1,0,1,0,0,0,0,0,0,0,1,0,1},
      {1,0,0,0,0,1,0,1,0,0,0,0,1},
      {1,0,1,1,0,0,0,0,0,1,1,0,1},
      {1,3,0,0,0,0,2,0,0,0,0,3,1},
      {1,1,1,1,1,1,1,1,1,1,1,1,1}
    };
    _dotsLeft = 0;
    for (int r = 0; r < 11; ++r)
      for (int c = 0; c < 13; ++c) {
        _maze[r][c] = tmpl[r][c];
        if (_maze[r][c] == 0 || _maze[r][c] == 3) _dotsLeft++;
      }
    _px = 6; _py = 9; _pdir = 0;
    _gx[0] = 5; _gy[0] = 5;
    _gx[1] = 7; _gy[1] = 5;
    _frightTimer = 0;
  }

  static void dirDelta(int8_t d, int8_t& dx, int8_t& dy) {
    dx = (d == 0) ? 1 : (d == 2 ? -1 : 0);
    dy = (d == 1) ? 1 : (d == 3 ? -1 : 0);
  }

public:
  const char* id() const override { return "pacmini"; }
  const char* title() const override { return "Pac-Mini"; }
  const char* desc() const override { return "Eat dots, dodge ghosts!"; }
  uint16_t themeColor() const override { return Color565::Yellow; }

  void init() override {
    _score = 0;
    _moveTimer = 0;
    _ghostTimer = 0;
    _frightTimer = 0;
    _animT = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    resetMaze();
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    _animT += dt;
    if (_frightTimer > 0) _frightTimer -= dt;

    if (input.leftPressed) _pdir = (_pdir + 3) & 3;
    else if (input.rightPressed) _pdir = (_pdir + 1) & 3;
    else if (input.okShortReleased) _pdir = (_pdir + 2) & 3;

    _moveTimer += dt;
    if (_moveTimer >= 0.18f) {
      _moveTimer = 0;
      int8_t dx, dy;
      dirDelta(_pdir, dx, dy);
      int8_t nx = _px + dx, ny = _py + dy;
      if (nx >= 0 && nx < 13 && ny >= 0 && ny < 11 && _maze[ny][nx] != 1) {
        _px = nx; _py = ny;
        if (_maze[_py][_px] == 0 || _maze[_py][_px] == 3) {
          if (_maze[_py][_px] == 3) {
            _frightTimer = 7.0f;
            _score += 25;
          } else {
            _score += 10;
          }
          _maze[_py][_px] = 2;
          _dotsLeft--;
          if (_dotsLeft == 0) { _score += 100; resetMaze(); }
        }
      }
    }

    _ghostTimer += dt;
    if (_ghostTimer >= 0.38f) {
      _ghostTimer = 0;
      for (int i = 0; i < 2; ++i) {
        int8_t valid[4]; int cnt = 0;
        for (int d = 0; d < 4; ++d) {
          int8_t dx, dy; dirDelta(d, dx, dy);
          int8_t nx = _gx[i] + dx, ny = _gy[i] + dy;
          if (nx >= 0 && nx < 13 && ny >= 0 && ny < 11 && _maze[ny][nx] != 1) valid[cnt++] = d;
        }
        if (cnt > 0) {
          int8_t d = valid[random(0, cnt)];
          int8_t dx, dy; dirDelta(d, dx, dy);
          _gx[i] += dx; _gy[i] += dy;
        }
      }
    }

    for (int i = 0; i < 2; ++i) {
      if (_gx[i] == _px && _gy[i] == _py) {
        if (_frightTimer > 0) {
          _score += 50;
          _gx[i] = 6; _gy[i] = 5;
        } else {
          triggerGameOver();
          return;
        }
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(4, 6, 16));
    int16_t ox = 5, oy = 15, cs = 9;
    bool blink = ((int)(_animT * 6) & 1) != 0;
    for (int r = 0; r < 11; ++r) {
      for (int c = 0; c < 13; ++c) {
        int16_t x = ox + c * cs, y = oy + r * cs;
        if (_maze[r][c] == 1) {
          gfx.fillRect(x, y, cs - 1, cs - 1, Color565::rgb(16, 36, 110));
          gfx.drawRect(x, y, cs - 1, cs - 1, Color565::NeonCyan);
        } else if (_maze[r][c] == 0) {
          gfx.fillCircle(x + 4, y + 4, 1, Color565::White);
        } else if (_maze[r][c] == 3 && blink) {
          gfx.fillCircle(x + 4, y + 4, 3, Color565::Gold);
        }
      }
    }

    // Pac-Man with chomping mouth
    int16_t pcx = ox + _px * cs + 4, pcy = oy + _py * cs + 4;
    gfx.fillCircle(pcx, pcy, 4, Color565::Yellow);
    if (blink) {
      int8_t dx, dy; dirDelta(_pdir, dx, dy);
      gfx.fillTriangle(pcx, pcy, pcx + dx * 5 - dy * 3, pcy + dy * 5 - dx * 3,
                       pcx + dx * 5 + dy * 3, pcy + dy * 5 + dx * 3, Color565::rgb(4, 6, 16));
    }

    // Ghosts
    const uint16_t gCols[2] = { Color565::Red, Color565::Magenta };
    for (int i = 0; i < 2; ++i) {
      int16_t gx = ox + _gx[i] * cs + 4, gy = oy + _gy[i] * cs + 4;
      uint16_t gc = (_frightTimer > 0) ? (blink ? Color565::SkyBlue : Color565::DeepBlue) : gCols[i];
      gfx.fillCircle(gx, gy - 1, 4, gc);
      gfx.fillRect(gx - 4, gy - 1, 8, 4, gc);
      gfx.drawPixel(gx - 2, gy - 1, Color565::White);
      gfx.drawPixel(gx + 2, gy - 1, Color565::White);
    }

    gfx.drawCenteredText(118, _frightTimer > 0 ? "EAT GHOSTS! (+50)" : "A:L  B:R  OK:180",
                         _frightTimer > 0 ? Color565::Lime : Color565::GrayLight);
    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
