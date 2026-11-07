// ============================================================================
// LyMini ESP32-C3 1.44" ST7735 MiniGames & Smart Idle TV v0.2.4
// 9-in-1 Arcade + Smart Idle TV + Power Manager + http://lymini.local
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "src/Config.h"
#include "src/SystemSettings.h"
#include "src/HighScoreStore.h"
#include "src/InputManager.h"
#include "src/DisplayCanvas.h"
#include "src/PowerManager.h"
#include "src/SampleMediaSeeder.h"
#include "src/GameRegistry.h"
#include "src/LauncherScene.h"
#include "src/IdleScene.h"
#include "src/WebPortal.h"
#include "src/Font5x7.h"

#define APP_VERSION "0.2.4"

// Global System Objects
SystemSettings gSettings;
DisplayCanvas  gDisplay;
InputManager   gInput;
PowerManager   gPower;
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
AppState gPrevState = STATE_LAUNCHER;
int8_t   gCurrentGameIdx = 0;
uint32_t gLastFrameTime = 0;

void setup() {
  Serial.begin(115200);

  // 1. Load Persistent Settings from NVS
  gSettings.begin();

  // 2. Init Buttons, Status LED, and Power/Battery Manager
  gInput.begin();
  gPower.begin();

  // 3. Init ST7735 Display & Framebuffer (Default Rotation 90° & BGR Color Order)
  gDisplay.begin();
  gDisplay.setRotation(gSettings.data.screen_rotation, gSettings.data.color_order_bgr != 0);
  gDisplay.setBrightness(gSettings.data.brightness);

  // 4. Seed Sample Photos (.rgb) & Videos (.mvid) into LittleFS on first boot
  SampleMediaSeeder::seedIfNeeded(gDisplay);

  // 5. Boot & Hardware RESET Policy (v0.2.3): Always open directly into Launcher Menu
  HighScoreStore::initRtc();
  HighScoreStore::setActiveGameIndex(TOTAL_GAMES); // Menu
  gLauncher.init(0);
  gState = STATE_LAUNCHER;

  // 6. Smart Wi-Fi Policy on Boot:
  // Turn Wi-Fi ON if we are in the Launcher Menu OR if NTP time is not yet synced
  if (gState == STATE_LAUNCHER || !gPower.isTimeSynced()) {
    gPower.requestWifiOn();
  }

  // 7. Start Web Portal Server (routes ready whenever Wi-Fi is active)
  gWebPortal.begin(&gDisplay, &gIdle);

  gLastFrameTime = micros();
}

void loop() {
  // 1. Service Web Portal requests (when Wi-Fi is active)
  gWebPortal.update();

  // Check if Web Portal requested immediate Live View / Media Preview on screen
  if (gWebPortal.consumeIdleViewRequest()) {
    if (gState != STATE_IDLE) {
      gPrevState = gState;
      gState = STATE_IDLE;
    }
    gInput.resetIdleTimer();
  }

  // 2. Frame pacing: 60 FPS (16.6ms) in Game/Menu, 24 FPS (41.6ms) in Idle to save power
  uint32_t targetFrameUs = (gState == STATE_IDLE) ? 41666UL : 16666UL;
  uint32_t nowMicros = micros();
  uint32_t deltaMicros = nowMicros - gLastFrameTime;
  if (deltaMicros < targetFrameUs) {
    if (gState == STATE_IDLE) delay(2);
    return;
  }
  gLastFrameTime = nowMicros;
  float dt = deltaMicros / 1000000.0f;
  if (dt > 0.05f) dt = 0.05f;

  // 3. Update Input & Power Manager
  // Hold BOOT for 7 seconds (7000 ms) in-game to return to menu, or 1.5s in Launcher to enter Idle
  uint32_t holdThreshold = (gState == STATE_GAME) ? BOOT_HOLD_MENU_EXIT_MS : BOOT_HOLD_LAUNCHER_MS;
  InputState input = gInput.update(holdThreshold);
  uint32_t idleSec = gInput.getIdleSeconds();

  gPower.update(gState == STATE_LAUNCHER,
                gState == STATE_GAME,
                gState == STATE_IDLE,
                idleSec,
                gDisplay);

  // Check inactivity timeout for Smart Idle TV Mode
  if (gState != STATE_IDLE && gSettings.data.idle_timeout_sec > 0) {
    if (idleSec >= gSettings.data.idle_timeout_sec) {
      gPrevState = gState;
      gIdle.init();
      gState = STATE_IDLE;
    }
  }

  // 4. State Machine Update & Render
  if (gState == STATE_LAUNCHER) {
    int8_t launchIdx = gLauncher.update(input, dt, gDisplay);
    if (launchIdx == LAUNCH_IDLE_TV) {
      // Held BOOT (1.5s) in Menu -> Enter Idle TV Mode immediately!
      gPrevState = STATE_LAUNCHER;
      gIdle.init();
      gState = STATE_IDLE;
      gIdle.render(gDisplay);
    } else if (launchIdx >= 0) {
      // Short-pressed BOOT in Menu -> Start selected game!
      gCurrentGameIdx = launchIdx;
      HighScoreStore::setActiveGameIndex(gCurrentGameIdx);
      gRegistry.getGame(gCurrentGameIdx)->init();
      gState = STATE_GAME;
      gRegistry.getGame(gCurrentGameIdx)->render(gDisplay);
    } else {
      gLauncher.render(gDisplay, input.okHoldPct);
    }

  } else if (gState == STATE_GAME) {
    // Hold BOOT for 4 seconds in Game -> Return to Launcher Menu & Turn Wi-Fi ON
    if (input.menuHold) {
      HighScoreStore::setActiveGameIndex(TOTAL_GAMES); // Menu
      gLauncher.init(gCurrentGameIdx);
      gState = STATE_LAUNCHER;
      gPower.requestWifiOn();
      gLauncher.render(gDisplay, 0);
    } else {
      IGame* game = gRegistry.getGame(gCurrentGameIdx);
      game->update(input, dt);
      game->render(gDisplay);
      // Show EXIT MENU countdown banner only after holding for >= 2.5 seconds (2500 ms)
      if (input.okHeldMs >= 2500 && input.okHoldRemMs > 0) {
        int16_t bw = ((input.okHeldMs - 2500) * 124) / 1500;
        if (bw > 124) bw = 124;
        gDisplay.fillRect(0, 118, SCREEN_WIDTH, 10, Color565::Black);
        gDisplay.fillRect(2, 119, bw, 8, Color565::Magenta);
        char hbuf[24];
        snprintf(hbuf, sizeof(hbuf), "EXIT MENU: %u.%us", input.okHoldRemMs / 1000, (input.okHoldRemMs % 1000) / 100);
        gDisplay.drawCenteredText(119, hbuf, Color565::White);
      }
    }

  } else if (gState == STATE_IDLE) {
    bool wakeUp = gIdle.update(input, dt);
    if (wakeUp) {
      gInput.resetIdleTimer();
      gState = gPrevState;
      if (gState == STATE_LAUNCHER) {
        gPower.requestWifiOn();
      }
    } else {
      gIdle.render(gDisplay);
    }
  }

  // 5. Push 128x128 RGB565 Framebuffer to ST7735 LCD over 40MHz SPI
  gDisplay.present();
}
