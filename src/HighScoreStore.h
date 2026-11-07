#pragma once
#include <Arduino.h>
#include <Preferences.h>

// Variable in RTC slow memory survives software resets and RESET button presses!
RTC_NOINIT_ATTR uint32_t gRtcMagic;
RTC_NOINIT_ATTR int32_t  gRtcGameIndex;

#define RTC_MAGIC_ARCADE 0xA8CA9E01

class HighScoreStore {
public:
  static void initRtc() {
    if (gRtcMagic != RTC_MAGIC_ARCADE) {
      gRtcMagic = RTC_MAGIC_ARCADE;
      gRtcGameIndex = 0; // Default to launcher
    }
  }

  static int32_t getActiveGameIndex() {
    return gRtcGameIndex;
  }

  static void setActiveGameIndex(int32_t idx) {
    gRtcGameIndex = idx;
  }

  static void nextGameOnReset(int32_t totalGamesPlusMenu) {
    initRtc();
    gRtcGameIndex = (gRtcGameIndex + 1) % totalGamesPlusMenu;
  }

  static uint32_t getHighScore(const char* gameId) {
    Preferences prefs;
    uint32_t hs = 0;
    char key[16];
    snprintf(key, sizeof(key), "hs_%s", gameId);
    if (prefs.begin("scores", true)) {
      hs = prefs.getUInt(key, 0);
      prefs.end();
    }
    return hs;
  }

  static bool submitScore(const char* gameId, uint32_t score) {
    Preferences prefs;
    char key[16];
    snprintf(key, sizeof(key), "hs_%s", gameId);
    bool isNewHigh = false;
    if (prefs.begin("scores", false)) {
      uint32_t cur = prefs.getUInt(key, 0);
      if (score > cur) {
        prefs.putUInt(key, score);
        isNewHigh = true;
      }
      prefs.end();
    }
    return isNewHigh;
  }

  static void resetAllScores() {
    Preferences prefs;
    if (prefs.begin("scores", false)) {
      prefs.clear();
      prefs.end();
    }
  }
};
