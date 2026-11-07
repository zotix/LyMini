#pragma once
#include <Arduino.h>
#include "IGame.h"
#include "games/GameDino.h"
#include "games/GamePong.h"
#include "games/GameSpaceRunner.h"
#include "games/GameCarDodge.h"
#include "games/GameSnake.h"
#include "games/GameTimberman.h"
#include "games/GameTetris.h"
#include "games/GameBreakout.h"
#include "games/GameBomberman.h"

#define TOTAL_GAMES 9

class GameRegistry {
private:
  GameDino        _gameDino;
  GamePong        _gamePong;
  GameSpaceRunner _gameSpace;
  GameCarDodge    _gameCar;
  GameSnake       _gameSnake;
  GameTimberman   _gameTimber;
  GameTetris      _gameTetris;
  GameBreakout    _gameBreakout;
  GameBomberman   _gameBomberman;

  IGame* _games[TOTAL_GAMES];

public:
  GameRegistry() {
    _games[0] = &_gameDino;
    _games[1] = &_gamePong;
    _games[2] = &_gameSpace;
    _games[3] = &_gameCar;
    _games[4] = &_gameSnake;
    _games[5] = &_gameTimber;
    _games[6] = &_gameTetris;
    _games[7] = &_gameBreakout;
    _games[8] = &_gameBomberman;
  }

  uint8_t getCount() const { return TOTAL_GAMES; }

  IGame* getGame(uint8_t index) {
    if (index >= TOTAL_GAMES) return _games[0];
    return _games[index];
  }
};
