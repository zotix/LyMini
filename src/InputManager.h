#pragma once
#include <Arduino.h>
#include "Config.h"
#include "SystemSettings.h"

struct InputState {
  bool leftPressed     = false; // Just pressed this frame
  bool rightPressed    = false;
  bool okPressed       = false;
  bool okShortReleased = false; // Released before hold threshold (1.2s)

  bool leftHeld        = false; // Currently being held down
  bool rightHeld       = false;
  bool okHeld          = false;

  bool bothPressed     = false; // Left + Right pressed together
  bool menuHold        = false; // OK held for > 1.2 seconds
  uint8_t okHoldPct    = 0;     // 0..100% progress toward menuHold
  bool anyActivity     = false; // Any button was touched or held
};

class InputManager {
private:
  bool _lastLeft = false;
  bool _lastRight = false;
  bool _lastOk = false;

  uint32_t _okPressStartTime = 0;
  bool _okHoldTriggered = false;

  uint32_t _lastActivityMs = 0;
  uint32_t _ledOffTimeMs = 0;

public:
  void begin() {
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
    _lastActivityMs = millis();
  }

  void triggerLed(uint32_t durationMs = 40) {
    if (!gSettings.data.led_enabled) return;
    digitalWrite(PIN_STATUS_LED, HIGH);
    _ledOffTimeMs = millis() + durationMs;
  }

  void resetIdleTimer() {
    _lastActivityMs = millis();
  }

  uint32_t getIdleSeconds() const {
    return (millis() - _lastActivityMs) / 1000UL;
  }

  InputState update(uint32_t holdThresholdMs = BOOT_HOLD_MENU_EXIT_MS) {
    InputState st;
    uint32_t now = millis();
    if (holdThresholdMs < 500) holdThresholdMs = 500;

    // Check LED timer
    if (_ledOffTimeMs > 0 && now >= _ledOffTimeMs) {
      digitalWrite(PIN_STATUS_LED, LOW);
      _ledOffTimeMs = 0;
    }

    // Active LOW buttons
    bool curLeft  = (digitalRead(PIN_BTN_LEFT) == LOW);
    bool curRight = (digitalRead(PIN_BTN_RIGHT) == LOW);
    bool curOk    = (digitalRead(PIN_BTN_OK) == LOW);

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
    } else if (curOk && !_okHoldTriggered) {
      uint32_t heldMs = now - _okPressStartTime;
      if (heldMs > 250) {
        uint32_t span = (holdThresholdMs > 250) ? (holdThresholdMs - 250) : 250;
        uint32_t p = ((heldMs - 250) * 100) / span;
        st.okHoldPct = (p > 100) ? 100 : (uint8_t)p;
      }
      if (heldMs >= holdThresholdMs) {
        st.menuHold = true;
        st.okHoldPct = 100;
        _okHoldTriggered = true;
        triggerLed(150);
      }
    } else if (!curOk && _lastOk) {
      if (!_okHoldTriggered && (now - _okPressStartTime < holdThresholdMs)) {
        st.okShortReleased = true;
      }
      _okHoldTriggered = false;
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
