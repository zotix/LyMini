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
#include "games/GameFlappy.h"
#include "games/Game2048.h"
#include "games/GameInvaders.h"
#include "games/GamePacMini.h"
#include "games/GameSimon.h"
#include "games/GameWhack.h"
#include "games/GameStack.h"
#include "games/GameFrogger.h"
#include "games/GameMines.h"

#define TOTAL_GAMES 18

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
  GameFlappy      _gameFlappy;
  Game2048        _game2048;
  GameInvaders    _gameInvaders;
  GamePacMini     _gamePacMini;
  GameSimon       _gameSimon;
  GameWhack       _gameWhack;
  GameStack       _gameStack;
  GameFrogger     _gameFrogger;
  GameMines       _gameMines;

  IGame* _games[TOTAL_GAMES];

public:
  GameRegistry() {
    _games[0]  = &_gameDino;
    _games[1]  = &_gamePong;
    _games[2]  = &_gameSpace;
    _games[3]  = &_gameCar;
    _games[4]  = &_gameSnake;
    _games[5]  = &_gameTimber;
    _games[6]  = &_gameTetris;
    _games[7]  = &_gameBreakout;
    _games[8]  = &_gameBomberman;
    _games[9]  = &_gameFlappy;
    _games[10] = &_game2048;
    _games[11] = &_gameInvaders;
    _games[12] = &_gamePacMini;
    _games[13] = &_gameSimon;
    _games[14] = &_gameWhack;
    _games[15] = &_gameStack;
    _games[16] = &_gameFrogger;
    _games[17] = &_gameMines;
  }

  uint8_t getCount() const { return TOTAL_GAMES; }

  IGame* getGame(uint8_t index) {
    if (index >= TOTAL_GAMES) return _games[0];
    return _games[index];
  }
};
