// ============================================================================
// Spotpear ESP32-C3 1.44" ST7735 MiniGames (8-in-1 Arcade + Smart Idle TV)
// Complete firmware supporting 60 FPS 16-bit RGB565 graphics, RESET switching,
// Idle 1-min screensaver, and http://mini.local Web Portal
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "src/Config.h"
#include "src/SystemSettings.h"
#include "src/HighScoreStore.h"
#include "src/InputManager.h"
#include "src/DisplayCanvas.h"
#include "src/GameRegistry.h"
#include "src/LauncherScene.h"
#include "src/IdleScene.h"
#include "src/WebPortal.h"
#include "src/Font5x7.h"

// Global System Objects
SystemSettings gSettings;
DisplayCanvas  gDisplay;
InputManager   gInput;
GameRegistry   gRegistry;
LauncherScene  gLauncher(gRegistry);
IdleScene      gIdle;
WebPortal      gWebPortal;

// System States
enum AppState {
  STATE_LAUNCHER,
  STATE_GAME,
  STATE_IDLE
};

AppState gState = STATE_LAUNCHER;
AppState gPrevState = STATE_LAUNCHER; // For resume after idle
int8_t   gCurrentGameIdx = 0;
uint32_t gLastFrameTime = 0;

void setup() {
  Serial.begin(115200);

  // 1. Load User Settings from NVS
  gSettings.begin();

  // 2. Init Buttons and Status LED
  gInput.begin();

  // 3. Init ST7735 Display & Framebuffer
  gDisplay.begin();
  gDisplay.setBrightness(gSettings.data.brightness);

  // 4. Check RESET button cause (Hardware Reset vs Power-On)
  HighScoreStore::initRtc();
  esp_reset_reason_t resetReason = esp_reset_reason();

  if (resetReason == ESP_RST_EXT || resetReason == ESP_RST_SW) {
    // Cycled via RESET button: switch to next game or menu!
    HighScoreStore::nextGameOnReset(TOTAL_GAMES + 1); // 0-7: Games, 8: Menu
  }

  int32_t activeIdx = HighScoreStore::getActiveGameIndex();
  if (activeIdx >= 0 && activeIdx < TOTAL_GAMES) {
    gCurrentGameIdx = activeIdx;
    gRegistry.getGame(gCurrentGameIdx)->init();
    gState = STATE_GAME;
  } else {
    gLauncher.init(0);
    gState = STATE_LAUNCHER;
  }

  // 5. Start Web Portal (Wi-Fi, mDNS http://mini.local, Cropper Web UI)
  gWebPortal.begin();

  // 6. Configure NTP time
  configTime(gSettings.data.timezone_offset * 3600, 0, "pool.ntp.org", "time.nist.gov");

  gLastFrameTime = micros();
}

void loop() {
  // 1. Maintain background web server requests
  gWebPortal.update();

  // 2. Calculate delta time (dt) targeting ~60 FPS
  uint32_t nowMicros = micros();
  uint32_t deltaMicros = nowMicros - gLastFrameTime;
  if (deltaMicros < 16666) { // ~60 FPS cap (16.6ms)
    return;
  }
  gLastFrameTime = nowMicros;
  float dt = deltaMicros / 1000000.0f;
  if (dt > 0.05f) dt = 0.05f; // Clamp large lags

  // 3. Update Input
  InputState input = gInput.update();

  // Check 1-minute inactivity timeout for Smart Idle TV Mode
  if (gState != STATE_IDLE && gSettings.data.idle_timeout_sec > 0) {
    if (gInput.getIdleSeconds() >= gSettings.data.idle_timeout_sec) {
      gPrevState = gState;
      gIdle.init();
      gState = STATE_IDLE;
    }
  }

  // 4. State Machine Update & Render
  if (gState == STATE_LAUNCHER) {
    int8_t launchIdx = gLauncher.update(input, dt);
    if (launchIdx >= 0) {
      gCurrentGameIdx = launchIdx;
      HighScoreStore::setActiveGameIndex(gCurrentGameIdx);
      gRegistry.getGame(gCurrentGameIdx)->init();
      gState = STATE_GAME;
    }
    gLauncher.render(gDisplay);

  } else if (gState == STATE_GAME) {
    // Hold BOOT for > 1.2s returns to Launcher Menu
    if (input.menuHold) {
      HighScoreStore::setActiveGameIndex(TOTAL_GAMES); // Menu
      gLauncher.init(gCurrentGameIdx);
      gState = STATE_LAUNCHER;
    } else {
      IGame* game = gRegistry.getGame(gCurrentGameIdx);
      game->update(input, dt);
      game->render(gDisplay);
    }

  } else if (gState == STATE_IDLE) {
    bool wakeUp = gIdle.update(input, dt);
    if (wakeUp) {
      gInput.resetIdleTimer();
      gState = gPrevState; // Resume gameplay!
    } else {
      gIdle.render(gDisplay);
    }
  }

  // 5. Transfer frame to LCD screen via 40MHz SPI
  gDisplay.present();
}
