#pragma once
#include <Arduino.h>
#include "../IGame.h"

class Game2048 : public IGame {
private:
  uint16_t _grid[4][4];
  bool _verticalAxis;

  void spawnTile() {
    uint8_t empty[16][2];
    uint8_t cnt = 0;
    for (int r = 0; r < 4; ++r)
      for (int c = 0; c < 4; ++c)
        if (_grid[r][c] == 0) { empty[cnt][0] = r; empty[cnt][1] = c; cnt++; }
    if (cnt > 0) {
      int pick = random(0, cnt);
      _grid[empty[pick][0]][empty[pick][1]] = (random(0, 10) == 0) ? 4 : 2;
    }
  }

  bool canMove() const {
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        if (_grid[r][c] == 0) return true;
        if (r < 3 && _grid[r][c] == _grid[r + 1][c]) return true;
        if (c < 3 && _grid[r][c] == _grid[r][c + 1]) return true;
      }
    }
    return false;
  }

  bool slideLine(uint16_t line[4]) {
    uint16_t tmp[4] = {0, 0, 0, 0};
    int k = 0;
    for (int i = 0; i < 4; ++i) if (line[i] != 0) tmp[k++] = line[i];
    for (int i = 0; i < 3; ++i) {
      if (tmp[i] != 0 && tmp[i] == tmp[i + 1]) {
        tmp[i] *= 2;
        _score += tmp[i];
        tmp[i + 1] = 0;
      }
    }
    uint16_t fin[4] = {0, 0, 0, 0};
    k = 0;
    for (int i = 0; i < 4; ++i) if (tmp[i] != 0) fin[k++] = tmp[i];
    bool moved = false;
    for (int i = 0; i < 4; ++i) {
      if (line[i] != fin[i]) moved = true;
      line[i] = fin[i];
    }
    return moved;
  }

  void moveDir(uint8_t dir) {
    bool any = false;
    for (int i = 0; i < 4; ++i) {
      uint16_t line[4];
      for (int j = 0; j < 4; ++j) {
        if (dir == 0) line[j] = _grid[i][j];
        else if (dir == 1) line[j] = _grid[i][3 - j];
        else if (dir == 2) line[j] = _grid[j][i];
        else line[j] = _grid[3 - j][i];
      }
      if (slideLine(line)) any = true;
      for (int j = 0; j < 4; ++j) {
        if (dir == 0) _grid[i][j] = line[j];
        else if (dir == 1) _grid[i][3 - j] = line[j];
        else if (dir == 2) _grid[j][i] = line[j];
        else _grid[3 - j][i] = line[j];
      }
    }
    if (any) {
      spawnTile();
      if (!canMove()) triggerGameOver();
    }
  }

  uint16_t tileColor(uint16_t v) const {
    switch (v) {
      case 0:    return Color565::rgb(24, 32, 48);
      case 2:    return Color565::rgb(64, 96, 140);
      case 4:    return Color565::rgb(42, 128, 152);
      case 8:    return Color565::rgb(230, 130, 40);
      case 16:   return Color565::rgb(220, 80, 50);
      case 32:   return Color565::Crimson;
      case 64:   return Color565::Magenta;
      case 128:  return Color565::Gold;
      case 256:  return Color565::Yellow;
      case 512:  return Color565::Lime;
      default:   return Color565::NeonCyan;
    }
  }

public:
  const char* id() const override { return "g2048"; }
  const char* title() const override { return "2048 Puzzle"; }
  const char* desc() const override { return "Merge tiles to 2048!"; }
  uint16_t themeColor() const override { return Color565::Orange; }

  void init() override {
    memset(_grid, 0, sizeof(_grid));
    _verticalAxis = false;
    _score = 0;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());
    spawnTile();
    spawnTile();
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.leftPressed || input.rightPressed || input.okShortReleased) init();
      return;
    }
    if (input.okShortReleased) {
      _verticalAxis = !_verticalAxis;
    } else if (input.leftPressed) {
      moveDir(_verticalAxis ? 2 : 0);
    } else if (input.rightPressed) {
      moveDir(_verticalAxis ? 3 : 1);
    }
  }

  void render(DisplayCanvas& gfx) override {
    gfx.fillGradientV(0, 13, SCREEN_WIDTH, SCREEN_HEIGHT - 13, Color565::rgb(12, 16, 28), Color565::rgb(20, 26, 44));
    gfx.drawRoundRect(11, 14, 106, 102, 4, _verticalAxis ? Color565::NeonCyan : Color565::Gold);

    int16_t startX = 14, startY = 17, cell = 23, gap = 2;
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        int16_t x = startX + c * (cell + gap);
        int16_t y = startY + r * (cell + gap);
        uint16_t v = _grid[r][c];
        uint16_t tc = tileColor(v);
        gfx.fillRoundRect(x, y, cell, cell, 3, tc);
        if (v > 0) {
          gfx.drawRoundRect(x, y, cell, cell, 3, Color565::White);
          char b[8];
          snprintf(b, sizeof(b), "%u", v);
          int16_t tx = x + (cell - (int16_t)strlen(b) * 6) / 2;
          gfx.drawText(tx, y + 8, b, (v == 128 || v == 256 || v == 512) ? Color565::Black : Color565::White);
        }
      }
    }

    gfx.fillRect(0, 117, SCREEN_WIDTH, 11, Color565::rgb(16, 22, 36));
    gfx.drawCenteredText(119, _verticalAxis ? "AXIS: ^/v  (OK:H/V)" : "AXIS: </>  (OK:H/V)",
                         _verticalAxis ? Color565::NeonCyan : Color565::Gold);
    drawHud(gfx);
    if (_isGameOver) drawGameOverOverlay(gfx);
  }
};
