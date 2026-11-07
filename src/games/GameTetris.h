#pragma once
#include <Arduino.h>
#include "../IGame.h"

class GameTetris : public IGame {
private:
  static const uint8_t BOARD_W = 10;
  static const uint8_t BOARD_H = 18;
  uint8_t _board[BOARD_H][BOARD_W]; // 0: empty, 1-7: piece color

  int8_t _curPiece;
  int8_t _curRot;
  int8_t _curX, _curY;
  int8_t _nextPiece;

  float _dropTimer;
  float _dropDelay;
  float _moveRepeatTimer;
  uint16_t _linesCleared;

  // Tetromino shapes definitions: 7 pieces, 4 rotations, 4x4 matrix
  // Stored as 16-bit bitmasks
  static const uint16_t PIECES[7][4];
  static const uint16_t PIECE_COLORS[8];

public:
  const char* id() const override { return "tetris"; }
  const char* title() const override { return "Cyber Tetris"; }
  const char* desc() const override { return "Classic 7-Piece Arcade!"; }
  uint16_t themeColor() const override { return Color565::Violet; }

  void init() override {
    for (int y = 0; y < BOARD_H; ++y) {
      for (int x = 0; x < BOARD_W; ++x) {
        _board[y][x] = 0;
      }
    }
    _score = 0;
    _linesCleared = 0;
    _dropTimer = 0.0f;
    _dropDelay = 0.85f; // Slower & easier falling speed
    _moveRepeatTimer = 0.0f;
    _isGameOver = false;
    _newHighScore = false;
    _highScore = HighScoreStore::getHighScore(id());

    _nextPiece = random(0, 7);
    spawnPiece();
  }

  bool checkCollision(int8_t piece, int8_t rot, int8_t px, int8_t py) {
    uint16_t mask = PIECES[piece][rot];
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        if (mask & (0x8000 >> (r * 4 + c))) {
          int8_t bx = px + c;
          int8_t by = py + r;
          if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return true;
          if (by >= 0 && _board[by][bx] != 0) return true;
        }
      }
    }
    return false;
  }

  void spawnPiece() {
    _curPiece = _nextPiece;
    _nextPiece = random(0, 7);
    _curRot = 0;
    _curX = (BOARD_W - 4) / 2;
    _curY = 0;

    if (checkCollision(_curPiece, _curRot, _curX, _curY)) {
      triggerGameOver();
    }
  }

  void lockPiece() {
    uint16_t mask = PIECES[_curPiece][_curRot];
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        if (mask & (0x8000 >> (r * 4 + c))) {
          int8_t bx = _curX + c;
          int8_t by = _curY + r;
          if (by >= 0 && by < BOARD_H && bx >= 0 && bx < BOARD_W) {
            _board[by][bx] = _curPiece + 1;
          }
        }
      }
    }

    // Check complete rows
    uint8_t clearedInStep = 0;
    for (int y = BOARD_H - 1; y >= 0; --y) {
      bool full = true;
      for (int x = 0; x < BOARD_W; ++x) {
        if (_board[y][x] == 0) {
          full = false;
          break;
        }
      }

      if (full) {
        clearedInStep++;
        for (int dy = y; dy > 0; --dy) {
          for (int x = 0; x < BOARD_W; ++x) {
            _board[dy][x] = _board[dy - 1][x];
          }
        }
        for (int x = 0; x < BOARD_W; ++x) _board[0][x] = 0;
        y++; // Check same row again
      }
    }

    if (clearedInStep > 0) {
      _linesCleared += clearedInStep;
      uint16_t linePoints[5] = { 0, 50, 120, 350, 1200 };
      _score += linePoints[clearedInStep];
      if (_dropDelay > 0.28f) _dropDelay -= (clearedInStep * 0.012f);
    }

    spawnPiece();
  }

  int8_t getGhostY() {
    int8_t gy = _curY;
    while (!checkCollision(_curPiece, _curRot, _curX, gy + 1)) {
      gy++;
    }
    return gy;
  }

  void update(const InputState& input, float dt) override {
    if (_isGameOver) {
      if (input.okPressed || input.leftPressed || input.rightPressed) {
        init();
      }
      return;
    }

    // Move Left / Right (supports both tap and smooth hold auto-repeat)
    _moveRepeatTimer -= dt;
    if (input.leftPressed || (input.leftHeld && !input.rightHeld && _moveRepeatTimer <= 0.0f)) {
      if (!checkCollision(_curPiece, _curRot, _curX - 1, _curY)) _curX--;
      _moveRepeatTimer = input.leftPressed ? 0.22f : 0.10f;
    }
    if (input.rightPressed || (input.rightHeld && !input.leftHeld && _moveRepeatTimer <= 0.0f)) {
      if (!checkCollision(_curPiece, _curRot, _curX + 1, _curY)) _curX++;
      _moveRepeatTimer = input.rightPressed ? 0.22f : 0.10f;
    }

    // Rotate Piece (OK Button short release so holding BOOT to exit doesn't rotate or drop!)
    if (input.okShortReleased && !input.bothPressed) {
      int8_t nextRot = (_curRot + 1) % 4;
      if (!checkCollision(_curPiece, nextRot, _curX, _curY)) {
        _curRot = nextRot;
      } else if (!checkCollision(_curPiece, nextRot, _curX - 1, _curY)) {
        _curX--;
        _curRot = nextRot;
      } else if (!checkCollision(_curPiece, nextRot, _curX + 1, _curY)) {
        _curX++;
        _curRot = nextRot;
      } else if (!checkCollision(_curPiece, nextRot, _curX - 2, _curY)) {
        _curX -= 2;
        _curRot = nextRot;
      }
    }

    // Hard Drop (Press Left + Right together)
    if (input.bothPressed) {
      _curY = getGhostY();
      lockPiece();
      return;
    }

    // Normal gravity drop ONLY (BOOT hold fast-drop disabled as requested!)
    _dropTimer += dt;
    if (_dropTimer >= _dropDelay) {
      _dropTimer = 0.0f;
      if (!checkCollision(_curPiece, _curRot, _curX, _curY + 1)) {
        _curY++;
      } else {
        lockPiece();
      }
    }
  }

  void render(DisplayCanvas& gfx) override {
    // Dark Backdrop
    gfx.fillRect(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, Color565::rgb(10, 12, 22));

    // Board Frame (X = 6, Y = 16, Cell = 6x6 px -> 10 cells = 60 px, 18 rows = 108 px)
    int16_t bx = 6;
    int16_t by = 16;
    gfx.drawRect(bx - 1, by - 1, (BOARD_W * 6) + 2, (BOARD_H * 6) + 2, Color565::DeepBlue);

    // Render Board Grid Cells
    for (int y = 0; y < BOARD_H; ++y) {
      for (int x = 0; x < BOARD_W; ++x) {
        uint8_t val = _board[y][x];
        if (val > 0) {
          uint16_t col = PIECE_COLORS[val];
          gfx.fillRect(bx + x * 6, by + y * 6, 5, 5, col);
          gfx.drawPixel(bx + x * 6, by + y * 6, Color565::White); // Highlight
        } else {
          // Subtle dot in grid center
          gfx.drawPixel(bx + x * 6 + 2, by + y * 6 + 2, Color565::rgb(20, 25, 35));
        }
      }
    }

    // Render Ghost Piece
    int8_t gy = getGhostY();
    uint16_t mask = PIECES[_curPiece][_curRot];
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        if (mask & (0x8000 >> (r * 4 + c))) {
          gfx.drawRect(bx + (_curX + c) * 6, by + (gy + r) * 6, 5, 5, Color565::GrayDark);
        }
      }
    }

    // Render Active Piece
    uint16_t pieceCol = PIECE_COLORS[_curPiece + 1];
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        if (mask & (0x8000 >> (r * 4 + c))) {
          int16_t px = bx + (_curX + c) * 6;
          int16_t py = by + (_curY + r) * 6;
          gfx.fillRect(px, py, 5, 5, pieceCol);
          gfx.drawPixel(px, py, Color565::White);
        }
      }
    }

    // Sidebar Info (NEXT, LINES)
    int16_t sx = 74;
    gfx.drawText(sx, 18, "NEXT", Color565::Cyan);
    gfx.drawRect(sx - 1, 28, 28, 28, Color565::Navy);

    // Render Next Piece preview
    uint16_t nextMask = PIECES[_nextPiece][0];
    uint16_t nextCol = PIECE_COLORS[_nextPiece + 1];
    for (int r = 0; r < 4; ++r) {
      for (int c = 0; c < 4; ++c) {
        if (nextMask & (0x8000 >> (r * 4 + c))) {
          gfx.fillRect(sx + 3 + c * 5, 31 + r * 5, 4, 4, nextCol);
        }
      }
    }

    gfx.drawText(sx, 65, "LINES", Color565::Gold);
    char lbuf[8];
    snprintf(lbuf, sizeof(lbuf), "%u", _linesCleared);
    gfx.drawText(sx, 77, lbuf, Color565::White);

    drawHud(gfx);

    if (_isGameOver) {
      drawGameOverOverlay(gfx);
    }
  }
};

// Tetromino bitmasks (I, J, L, O, S, T, Z)
const uint16_t GameTetris::PIECES[7][4] = {
  { 0x0F00, 0x2222, 0x00F0, 0x4444 }, // I
  { 0x8E00, 0x6440, 0x0E20, 0x44C0 }, // J
  { 0x2E00, 0x4460, 0x0E80, 0xC440 }, // L
  { 0x6600, 0x6600, 0x6600, 0x6600 }, // O
  { 0x6C00, 0x4620, 0x06C0, 0x8C40 }, // S
  { 0x4E00, 0x4640, 0x0E40, 0x4C40 }, // T
  { 0xC600, 0x2640, 0x0C60, 0x4C80 }  // Z
};

const uint16_t GameTetris::PIECE_COLORS[8] = {
  Color565::Black,
  Color565::Cyan,    // I
  Color565::Blue,    // J
  Color565::Orange,  // L
  Color565::Yellow,  // O
  Color565::Lime,    // S
  Color565::Magenta, // T
  Color565::Red      // Z
};
