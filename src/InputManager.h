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
  uint8_t  okHoldPct   = 0;     // 0..100% progress toward menuHold
  uint16_t okHoldRemMs = 0;     // Remaining ms until menuHold triggers
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

  uint32_t _lastActivityMs = 0;
  uint32_t _ledOffTimeMs = 0;

  static inline uint32_t nowRealMs() {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
  }

public:
  void begin() {
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
    _lastActivityMs = nowRealMs();
  }

  void triggerLed(uint32_t durationMs = 40) {
    if (!gSettings.data.led_enabled) return;
    digitalWrite(PIN_STATUS_LED, HIGH);
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
      digitalWrite(PIN_STATUS_LED, LOW);
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
    } else if (curOk && !_okHoldTriggered) {
      uint32_t heldMs = now - _okPressStartTime;
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
