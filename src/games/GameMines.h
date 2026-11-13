#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameMines : public IGame {
private:
  uint8_t _mine[6][6];   // 1 = mine
  uint8_t _state[6][6];  // 0 = hidden, 1 = revealed, 2 = flagged
  uint8_t _adj[6][6];
  int8_t _cur;           // 0..35
  bool _won;

  void buildBoard() {
    memset(_mine, 0, sizeof(_mine));
    memset(_state, 0, sizeof(_state));
    memset(_adj, 0, sizeof(_adj));
    int placed = 0;
    while (placed < 6) {
      int r = random(0, 6), c = random(0, 6);
      if (!_mine[r][c]) { _mine[r][c] = 1; placed++; }
    }
    for (int r = 0; r < 6; ++r) {
      for (int c = 0; c < 6; ++c) {
        uint8_t cnt = 0;
        for (int dr = -1; dr <= 1; ++dr)
          for (int dc = -1; dc <= 1; ++dc) {
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < 6 && nc >= 0 && nc < 6 && _mine[nr][nc]) cnt++;
          }
        _adj[r][c] = cnt;
      }
    }
  }

  void reveal(int r, int c) {
    if (r < 0 || r >= 6 || c < 0 || c >= 6 || _state[r][c] != 0) return;
    _state[r][c] = 1;
    if (_mine[r][c]) {
      for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 6; ++x)
          if (_mine[y][x]) _state[y][x] = 1;
      triggerGameOver();
      return;
    }
    _score += 10;
    if (_adj[r][c] == 0) {
      for (int dr = -1; dr <= 1; ++dr)
        for (int dc = -1; dc <= 1; ++dc)
          if (dr != 0 || dc != 0) reveal(r + dr, c + dc);
    }
    int hiddenSafe = 0;
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 6; ++x)
        if (!_mine[y][x] && _state[y][x] != 1) hiddenSafe++;
    if (hiddenSafe == 0) {
      _score += 100;
      _won = true;
      buildBoard();
    }
  }

public:
  const char* id() const override { return "mines"; }
  const char* title() const override { return "Minesweeper"; }
  const char* desc() const override { return "Clear safe tiles!"; }
  uint16_t themeColor() const override { return Color565::SkyBlue; }

  void init() override {
    _cur = 0;
    _won = false;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    buildBoard();
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }

    if (input.leftPressed) {
      _cur = (_cur + 1) % 36;
    } else if (input.rightPressed) {
      int r = _cur / 6, c = _cur % 6;
      if (_state[r][c] == 0) _state[r][c] = 2;
      else if (_state[r][c] == 2) _state[r][c] = 0;
    } else if (input.okShortReleased) {
      int r = _cur / 6, c = _cur % 6;
      if (_state[r][c] == 0) reveal(r, c);
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillRect(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(12, 18, 30));
    int16_t ox = 16, oy = 17, cs = 16;
    for (int r = 0; r < 6; ++r) {
      for (int c = 0; c < 6; ++c) {
        int16_t x = ox + c * cs, y = oy + r * cs;
        uint8_t st = _state[r][c];
        if (st == 0) {
          gfx.fillRect(x + 1, y + 1, cs - 2, cs - 2, Color565::GrayMid);
        } else if (st == 2) {
          gfx.fillRect(x + 1, y + 1, cs - 2, cs - 2, Color565::Navy);
          gfx.drawText(x + 5, y + 4, "F", Color565::Gold);
        } else if (_mine[r][c]) {
          gfx.fillRect(x + 1, y + 1, cs - 2, cs - 2, Color565::Crimson);
          gfx.drawText(x + 5, y + 4, "*", Color565::White);
        } else {
          gfx.fillRect(x + 1, y + 1, cs - 2, cs - 2, Color565::rgb(26, 36, 54));
          if (_adj[r][c] > 0) {
            char b[4]; snprintf(b, sizeof(b), "%u", _adj[r][c]);
            gfx.drawText(x + 5, y + 4, b, _adj[r][c] == 1 ? Color565::NeonCyan : Color565::Lime);
          }
        }
        if ((r * 6 + c) == _cur) {
          gfx.drawRect(x, y, cs, cs, Color565::Gold);
        }
      }
    }
    gfx.drawCenteredText(118, "A:NEXT  B:FLAG  OK:DIG", Color565::SkyBlue);
    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
