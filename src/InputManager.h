#pragma once
#include <Arduino.h>
#include "esp_timer.h"
#include "Config.h"
#include "SystemSettings.h"

struct InputState {
  bool leftPressed     = false; // Just pressed this frame
  bool rightPressed    = false;
  bool okPressed       = false;
  bool okShortReleased = false; // Released before hold threshold

  bool leftHeld        = false; // Currently being held down
  bool rightHeld       = false;
  bool okHeld          = false;

  bool bothPressed     = false; // Left + Right pressed together
  bool menuHold        = false; // OK held for >= holdThresholdMs
  bool okHold10s       = false; // OK held for >= 10000 ms (OTA Update Confirmation)
  uint8_t  okHoldPct   = 0;     // 0..100% progress toward menuHold
  uint16_t okHoldRemMs = 0;     // Remaining ms until menuHold triggers
  uint16_t okHold10sRemMs = 0;  // Remaining ms until 10s OTA triggers
  uint16_t okHeldMs    = 0;     // Milliseconds OK button has been continuously held

  bool btnAHold5s      = false; // Button A held for >= 5000 ms (Enter Eco)
  uint8_t  btnAHoldPct = 0;     // 0..100% progress toward 5s
  uint16_t btnAHoldRemMs = 0;   // Remaining ms until 5s
  uint16_t btnAHeldMs  = 0;     // Milliseconds Button A has been continuously held

  bool btnBHold5s      = false; // Button B held for >= 5000 ms (Show Coffee QR)
  uint8_t  btnBHoldPct = 0;     // 0..100% progress toward 5s
  uint16_t btnBHoldRemMs = 0;   // Remaining ms until 5s
  uint16_t btnBHeldMs  = 0;     // Milliseconds Button B has been continuously held

  bool anyActivity     = false; // Any button was touched or held
};

class InputManager {
private:
  bool _lastLeft = false;
  bool _lastRight = false;
  bool _lastOk = false;

  uint32_t _okPressStartTime = 0;
  uint32_t _okLastLowTime = 0;
  bool _okHoldTriggered = false;
  bool _okHold10sTriggered = false;

  uint32_t _leftPressStartTime = 0;
  bool _leftHoldTriggered = false;

  uint32_t _rightPressStartTime = 0;
  bool _rightHoldTriggered = false;

  uint32_t _lastActivityMs = 0;
  uint32_t _ledOffTimeMs = 0;
  bool     _wifiLedState = false;

  static inline uint32_t nowRealMs() {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
  }

public:
  void begin() {
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, _wifiLedState ? HIGH : LOW);
    _lastActivityMs = nowRealMs();
  }

  void setWifiLedState(bool on) {
    if (_wifiLedState != on) {
      _wifiLedState = on;
      if (_ledOffTimeMs == 0) {
        digitalWrite(PIN_STATUS_LED, _wifiLedState ? HIGH : LOW);
      }
    }
  }

  void triggerLed(uint32_t durationMs = 40) {
    if (!gSettings.data.led_enabled) return;
    digitalWrite(PIN_STATUS_LED, _wifiLedState ? LOW : HIGH);
    _ledOffTimeMs = nowRealMs() + durationMs;
  }

  void resetIdleTimer() {
    _lastActivityMs = nowRealMs();
  }

  uint32_t getIdleSeconds() const {
    return (nowRealMs() - _lastActivityMs) / 1000UL;
  }

  InputState update(uint32_t holdThresholdMs = BOOT_HOLD_MENU_EXIT_MS) {
    InputState st;
    uint32_t now = nowRealMs();
    if (holdThresholdMs < 500) holdThresholdMs = 500;

    // Check LED timer
    if (_ledOffTimeMs > 0 && now >= _ledOffTimeMs) {
      digitalWrite(PIN_STATUS_LED, _wifiLedState ? HIGH : LOW);
      _ledOffTimeMs = 0;
    }

    // Active LOW buttons
    bool rawLeft  = (digitalRead(PIN_BTN_LEFT) == LOW);
    bool rawRight = (digitalRead(PIN_BTN_RIGHT) == LOW);
    bool rawOk    = (digitalRead(PIN_BTN_OK) == LOW);

    // Debounce BOOT (OK) button against micro-bounce while holding for 4 seconds:
    // Keep curOk true for 80ms after the last LOW reading so contact chatter never resets the hold timer!
    if (rawOk) {
      _okLastLowTime = now;
    }
    bool curOk = rawOk || (_lastOk && (now - _okLastLowTime < 80));

    // Rotate Left/Right buttons with screen orientation when enabled:
    // Standard orientation is 270 deg (rot == 3) or 0 deg (rot == 0).
    // When flipped 180 deg to 90 deg (rot == 1) or 180 deg (rot == 2), swap Left & Right!
    bool swapLR = (gSettings.data.btn_rotate_with_screen != 0) &&
                  (gSettings.data.screen_rotation == 1 || gSettings.data.screen_rotation == 2);
    bool curLeft  = swapLR ? rawRight : rawLeft;
    bool curRight = swapLR ? rawLeft  : rawRight;

    st.leftHeld  = curLeft;
    st.rightHeld = curRight;
    st.okHeld    = curOk;

    st.leftPressed  = (curLeft && !_lastLeft);
    st.rightPressed = (curRight && !_lastRight);
    st.okPressed    = (curOk && !_lastOk);

    st.bothPressed = (st.leftPressed && curRight) || (st.rightPressed && curLeft);

    if (curOk && !_lastOk) {
      _okPressStartTime = now;
      _okHoldTriggered = false;
      _okHold10sTriggered = false;
    } else if (curOk) {
      uint32_t heldMs = now - _okPressStartTime;
      st.okHeldMs = (heldMs > 65535) ? 65535 : (uint16_t)heldMs;
      if (!_okHoldTriggered) {
        if (heldMs > 200) {
          uint32_t p = (heldMs * 100) / holdThresholdMs;
          st.okHoldPct = (p > 100) ? 100 : (uint8_t)p;
          st.okHoldRemMs = (heldMs < holdThresholdMs) ? (uint16_t)(holdThresholdMs - heldMs) : 0;
        }
        if (heldMs >= holdThresholdMs) {
          st.menuHold = true;
          st.okHoldPct = 100;
          st.okHoldRemMs = 0;
          _okHoldTriggered = true;
          triggerLed(150);
        }
      }
      if (!_okHold10sTriggered) {
        st.okHold10sRemMs = (heldMs < BOOT_HOLD_OTA_MS) ? (uint16_t)(BOOT_HOLD_OTA_MS - heldMs) : 0;
        if (heldMs >= BOOT_HOLD_OTA_MS) {
          st.okHold10s = true;
          st.okHold10sRemMs = 0;
          _okHold10sTriggered = true;
          triggerLed(250);
        }
      }
    } else if (!curOk && _lastOk) {
      if (!_okHoldTriggered && (now - _okPressStartTime < holdThresholdMs)) {
        st.okShortReleased = true;
      }
      _okHoldTriggered = false;
      _okHold10sTriggered = false;
    }

    // Button A (IO10) Hold 5s for Eco Mode
    if (rawLeft && !_lastLeft) {
      _leftPressStartTime = now;
      _leftHoldTriggered = false;
    } else if (rawLeft && !_leftHoldTriggered) {
      uint32_t heldMs = now - _leftPressStartTime;
      st.btnAHeldMs = (heldMs > 65535) ? 65535 : (uint16_t)heldMs;
      if (heldMs > 200) {
        uint32_t p = (heldMs * 100) / 5000;
        st.btnAHoldPct = (p > 100) ? 100 : (uint8_t)p;
        st.btnAHoldRemMs = (heldMs < 5000) ? (uint16_t)(5000 - heldMs) : 0;
      }
      if (heldMs >= 5000) {
        st.btnAHold5s = true;
        st.btnAHoldPct = 100;
        st.btnAHoldRemMs = 0;
        _leftHoldTriggered = true;
        triggerLed(150);
      }
    } else if (!rawLeft && _lastLeft) {
      _leftHoldTriggered = false;
    }

    // Button B (IO8) Hold 5s for Buy Me a Coffee QR
    if (rawRight && !_lastRight) {
      _rightPressStartTime = now;
      _rightHoldTriggered = false;
    } else if (rawRight && !_rightHoldTriggered) {
      uint32_t heldMs = now - _rightPressStartTime;
      st.btnBHeldMs = (heldMs > 65535) ? 65535 : (uint16_t)heldMs;
      if (heldMs > 200) {
        uint32_t p = (heldMs * 100) / 5000;
        st.btnBHoldPct = (p > 100) ? 100 : (uint8_t)p;
        st.btnBHoldRemMs = (heldMs < 5000) ? (uint16_t)(5000 - heldMs) : 0;
      }
      if (heldMs >= 5000) {
        st.btnBHold5s = true;
        st.btnBHoldPct = 100;
        st.btnBHoldRemMs = 0;
        _rightHoldTriggered = true;
        triggerLed(150);
      }
    } else if (!rawRight && _lastRight) {
      _rightHoldTriggered = false;
    }

    _lastLeft  = curLeft;
    _lastRight = curRight;
    _lastOk    = curOk;

    if (curLeft || curRight || curOk || st.leftPressed || st.rightPressed || st.okPressed || st.okShortReleased) {
      _lastActivityMs = now;
      st.anyActivity = true;
    }

    return st;
  }
};
