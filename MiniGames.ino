// ============================================================================
// LyMini ESP32-C3 1.44" ST7735 MiniGames & Smart Idle TV v0.2.6
// 9-in-1 Arcade + Smart Idle TV + Power Manager + http://lymini.local
// ============================================================================

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
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
#include "src/WifiQrOverlay.h"
#include "src/OtaUpdater.h"
#include "src/PromptPayQrAsset.h"
#include "src/Font5x7.h"

#ifndef APP_VERSION
#define APP_VERSION "0.2.6"
#endif

// Global System Objects
SystemSettings gSettings;
DisplayCanvas  gDisplay;
InputManager   gInput;
PowerManager   gPower;
GameRegistry   gRegistry;
LauncherScene  gLauncher(gRegistry);
IdleScene      gIdle;
WebPortal      gWebPortal;
WifiQrOverlay  gWifiQr;
OtaUpdater     gOta;

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
bool     gShowCoffeeQr = false;

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

  // 2. Frame pacing:
  //    - Active Game / Menu: 60 FPS (16.6ms) @ 160 MHz
  //    - Idle Display: 24 FPS (41.6ms) @ 40 MHz
  //    - Eco Display (>= 5 min): 15 FPS (66.6ms) @ 20 MHz
  //    - Sleep Mode (>= 45 min): 0 FPS @ 20 MHz (delay 50ms)
  uint32_t targetFrameUs = 16666UL;
  if (gState == STATE_IDLE) {
    if (gPower.isSleepMode(gInput.getIdleSeconds())) {
      targetFrameUs = 100000UL; // Low-frequency pacing in Sleep mode
    } else if (gPower.isEcoMode(gInput.getIdleSeconds())) {
      targetFrameUs = 66666UL; // 15 FPS in Eco mode (20 MHz)
    } else {
      targetFrameUs = 41666UL; // 24 FPS in Idle mode (40 MHz)
    }
  }
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

  // Sync GPIO11 Status LED with Wi-Fi power state
  gInput.setWifiLedState(gPower.isWifiEnabled());

  // Global Modal: OTA Update Confirmation & Progress Screen
  if (gOta.isModalActive()) {
    gInput.resetIdleTimer();
    gOta.updateAndRender(input, gDisplay);
    return;
  }

  // Global Modal: Buy Me a Coffee / PromptPay QR Screen
  if (gShowCoffeeQr) {
    if (input.leftPressed || input.rightPressed || input.okPressed || input.okShortReleased) {
      gShowCoffeeQr = false;
      gInput.resetIdleTimer();
    } else {
      renderPromptPayCoffeeQr(gDisplay);
      gDisplay.present();
      return;
    }
  }

  // Global Shortcut: Hold BOOT (10s) -> Open OTA Update Confirmation Screen
  if (input.okHold10s) {
    if (gDisplay.isSleeping()) gDisplay.wakeDisplay();
    gOta.openConfirmModal();
    gOta.updateAndRender(input, gDisplay);
    return;
  }

  // Global Shortcut: Hold Button A (5s) -> Enter Eco Mode immediately
  if (input.btnAHold5s) {
    gPrevState = (gState != STATE_IDLE) ? gState : STATE_LAUNCHER;
    gIdle.init();
    gState = STATE_IDLE;
    gPower.forceEcoMode();
  }

  // Global Shortcut: Hold Button B (5s) -> Show Buy Me a Coffee QR Code
  if (input.btnBHold5s) {
    gShowCoffeeQr = true;
  }

  // Clear forced Eco mode only when a NEW press occurs (not holding)
  if ((input.leftPressed || input.rightPressed || input.okPressed || input.okShortReleased) && gPower.isForceEco()) {
    gPower.clearForceEco();
    gInput.resetIdleTimer();
  }

  gPower.update(gState == STATE_LAUNCHER,
                gState == STATE_GAME,
                gState == STATE_IDLE,
                idleSec,
                gDisplay);

  // Monitor Wi-Fi connection and show 5-second QR code overlay
  gWifiQr.checkWifiStatus();
  if (input.anyActivity) {
    gWifiQr.dismiss();
  }

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
    bool isSleep = gPower.isSleepMode(idleSec);
    if (isSleep) {
      if (!gDisplay.isSleeping()) {
        gIdle.closeActiveVideo();
        gDisplay.clear(Color565::Black);
        gDisplay.present();
        gDisplay.sleepDisplay();
        gPower.requestWifiOff();
      }
      if (input.anyActivity) {
        gDisplay.wakeDisplay();
        gInput.resetIdleTimer();
        gState = gPrevState;
        if (gState == STATE_LAUNCHER) {
          gPower.requestWifiOn();
        }
      } else {
        // ESP32-C3 Light Sleep (<1 mA) with GPIO wakeup on any button or 250ms timer
        gpio_wakeup_enable((gpio_num_t)PIN_BTN_LEFT, GPIO_INTR_LOW_LEVEL);
        gpio_wakeup_enable((gpio_num_t)PIN_BTN_RIGHT, GPIO_INTR_LOW_LEVEL);
        gpio_wakeup_enable((gpio_num_t)PIN_BTN_OK, GPIO_INTR_LOW_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        esp_sleep_enable_timer_wakeup(250000ULL);
        esp_light_sleep_start();
      }
    } else {
      if (gDisplay.isSleeping()) {
        gDisplay.wakeDisplay();
      }
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
  }

  // 4b. Wi-Fi Connected 5-second QR code overlay
  if (gWifiQr.isActive() && gState != STATE_GAME) {
    gWifiQr.render(gDisplay);
  }

  // 4c. Global Button Hold Countdown Banners (Button A: Eco Mode / Button B: Buy Coffee / BOOT 10s: OTA)
  if (input.okHeldMs >= 5000 && input.okHold10sRemMs > 0 && !input.okHold10s) {
    int16_t bw = ((input.okHeldMs - 5000) * 124) / 5000;
    if (bw > 124) bw = 124;
    gDisplay.fillRect(0, 118, SCREEN_WIDTH, 10, Color565::Black);
    gDisplay.fillRect(2, 119, bw, 8, Color565::Cyan);
    char hbuf[24];
    snprintf(hbuf, sizeof(hbuf), "OTA UPDATE: %u.%us", input.okHold10sRemMs / 1000, (input.okHold10sRemMs % 1000) / 100);
    gDisplay.drawCenteredText(119, hbuf, Color565::White);
  } else if (input.btnAHeldMs >= 1500 && input.btnAHoldRemMs > 0 && !input.btnAHold5s) {
    int16_t bw = ((input.btnAHeldMs - 1500) * 124) / 3500;
    if (bw > 124) bw = 124;
    gDisplay.fillRect(0, 118, SCREEN_WIDTH, 10, Color565::Black);
    gDisplay.fillRect(2, 119, bw, 8, Color565::Green);
    char hbuf[24];
    snprintf(hbuf, sizeof(hbuf), "ECO MODE: %u.%us", input.btnAHoldRemMs / 1000, (input.btnAHoldRemMs % 1000) / 100);
    gDisplay.drawCenteredText(119, hbuf, Color565::White);
  } else if (input.btnBHeldMs >= 1500 && input.btnBHoldRemMs > 0 && !input.btnBHold5s) {
    int16_t bw = ((input.btnBHeldMs - 1500) * 124) / 3500;
    if (bw > 124) bw = 124;
    gDisplay.fillRect(0, 118, SCREEN_WIDTH, 10, Color565::Black);
    gDisplay.fillRect(2, 119, bw, 8, Color565::Gold);
    char hbuf[24];
    snprintf(hbuf, sizeof(hbuf), "BUY COFFEE: %u.%us", input.btnBHoldRemMs / 1000, (input.btnBHoldRemMs % 1000) / 100);
    gDisplay.drawCenteredText(119, hbuf, Color565::Black);
  }

  // 5. Push 128x128 RGB565 Framebuffer to ST7735 LCD over 40MHz SPI
  if (gState == STATE_IDLE) {
    gDisplay.presentIfChanged();
  } else {
    gDisplay.present();
  }
}
