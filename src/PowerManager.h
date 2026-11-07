#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <time.h>
#include "driver/usb_serial_jtag.h"
#include "Config.h"
#include "SystemSettings.h"
#include "DisplayCanvas.h"

RTC_NOINIT_ATTR uint32_t gRtcPwrMagic;
RTC_NOINIT_ATTR uint16_t gRtcBattPermille; // 0..1000 (100.0%)

#define RTC_PWR_MAGIC 0x50575232
#define WIFI_SEARCH_TIMEOUT_MS 45000UL

enum WifiPowerState {
  WIFI_PWR_OFF = 0,
  WIFI_PWR_SEARCHING,
  WIFI_PWR_STA_CONNECTED,
  WIFI_PWR_AP_ACTIVE
};

class PowerManager {
private:
  bool _wifiEnabled = false;
  bool _mdnsStarted = false;
  uint32_t _wifiTurnedOnMs = 0;
  uint32_t _lastWebActivityMs = 0;
  uint32_t _lastWifiTryMs = 0;
  uint8_t  _curWifiSlot = 0;

  uint32_t _lastBattUpdateMs = 0;
  bool     _hasExternalAdc = false;
  uint8_t  _batteryPct = 100;
  bool     _usbConnected = true;
  uint8_t  _currentCpuMhz = 160;
  uint8_t  _appliedBrightness = 85;

  void detectExternalAdc() {
    // Check if GPIO 1 (P111) has an external battery divider connected
    // If unconnected, internal pull-up vs pull-down will swing the pin from ~0 to ~4095
    pinMode(1, INPUT_PULLUP);
    delay(2);
    int vHigh = analogRead(1);
    pinMode(1, INPUT_PULLDOWN);
    delay(2);
    int vLow = analogRead(1);
    pinMode(1, INPUT);
    // If external low-impedance divider is attached, vHigh and vLow will be close
    if (abs(vHigh - vLow) < 600 && vLow > 800 && vHigh < 3600) {
      _hasExternalAdc = true;
    } else {
      _hasExternalAdc = false;
    }
  }

  void setCpuSpeed(uint8_t mhz) {
    if (_currentCpuMhz == mhz) return;
    // ESP32-C3 requires 160MHz while Wi-Fi radio is enabled
    if (_wifiEnabled && mhz < 160) mhz = 160;
    setCpuFrequencyMhz(mhz);
    _currentCpuMhz = mhz;
  }

public:
  void begin() {
    if (gRtcPwrMagic != RTC_PWR_MAGIC) {
      gRtcPwrMagic = RTC_PWR_MAGIC;
      gRtcBattPermille = 960; // Start at 96% when first powered
    }
    if (gRtcBattPermille > 1000) gRtcBattPermille = 1000;
    if (gRtcBattPermille < 50)   gRtcBattPermille = 50;

    detectExternalAdc();
    updateBattery();
  }

  bool isTimeSynced() const {
    time_t now = time(nullptr);
    return (now > 1700000000); // Year >= 2023
  }

  bool isWifiEnabled() const { return _wifiEnabled; }

  WifiPowerState getWifiState() const {
    if (!_wifiEnabled) return WIFI_PWR_OFF;
    if (WiFi.status() == WL_CONNECTED) return WIFI_PWR_STA_CONNECTED;
    if (WiFi.softAPgetStationNum() > 0) return WIFI_PWR_AP_ACTIVE;
    return WIFI_PWR_SEARCHING;
  }

  uint8_t getBatteryPercent() const { return _batteryPct; }
  bool isUsbOrCharging() const { return _usbConnected; }
  uint8_t getCpuMhz() const { return _currentCpuMhz; }

  void notifyWebActivity() {
    _lastWebActivityMs = millis();
    if (!_wifiEnabled) {
      requestWifiOn();
    }
  }

  void requestWifiOn() {
    uint32_t now = millis();
    _wifiTurnedOnMs = now;
    _lastWebActivityMs = now;

    if (_wifiEnabled) return;

    setCpuSpeed(160);
    _wifiEnabled = true;

    if (gSettings.data.wifi_count > 0) {
      WiFi.mode(WIFI_AP_STA);
      _curWifiSlot = 0;
      WiFi.begin(gSettings.data.wifi_list[0].ssid, gSettings.data.wifi_list[0].pass);
      _lastWifiTryMs = now;
    } else {
      WiFi.mode(WIFI_AP);
    }

    WiFi.softAP("Spotpear-Mini", "12345678");

    if (!_mdnsStarted) {
      if (MDNS.begin("mini")) {
        MDNS.addService("http", "tcp", 80);
        _mdnsStarted = true;
      }
    }

    configTime(gSettings.data.timezone_offset * 3600, 0, "pool.ntp.org", "time.nist.gov");
  }

  void requestWifiOff() {
    if (!_wifiEnabled) return;
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
    _wifiEnabled = false;
  }

  void toggleWifi() {
    if (_wifiEnabled) {
      requestWifiOff();
    } else {
      requestWifiOn();
    }
  }

  void updateBattery() {
    _usbConnected = usb_serial_jtag_is_connected();

    if (_hasExternalAdc) {
      uint32_t mv = analogReadMilliVolts(1) * 2; // Assuming 1:1 divider
      if (mv >= 4150) _batteryPct = 100;
      else if (mv <= 3300) _batteryPct = 5;
      else _batteryPct = (uint8_t)(((mv - 3300) * 95) / 850 + 5);
      gRtcBattPermille = _batteryPct * 10;
    } else {
      if (_usbConnected) {
        if (gRtcBattPermille < 1000) gRtcBattPermille += 2;
        if (gRtcBattPermille > 1000) gRtcBattPermille = 1000;
      } else {
        // Estimate discharge rate based on Wi-Fi & backlight load
        uint16_t drain = _wifiEnabled ? 2 : 1;
        if (gRtcBattPermille > 50 + drain) gRtcBattPermille -= drain;
      }
      _batteryPct = (uint8_t)(gRtcBattPermille / 10);
    }
  }

  void update(bool inMenu, bool inGame, bool inIdle, uint32_t idleSec, DisplayCanvas& display) {
    uint32_t now = millis();

    // 1. Periodic Battery Check (every 5s)
    if (now - _lastBattUpdateMs >= 5000) {
      _lastBattUpdateMs = now;
      updateBattery();
    }

    // 2. Multi-WiFi cycling while searching
    if (_wifiEnabled && WiFi.status() != WL_CONNECTED && gSettings.data.wifi_count > 1) {
      if (now - _lastWifiTryMs >= 10000) {
        _lastWifiTryMs = now;
        _curWifiSlot = (_curWifiSlot + 1) % gSettings.data.wifi_count;
        WiFi.begin(gSettings.data.wifi_list[_curWifiSlot].ssid,
                   gSettings.data.wifi_list[_curWifiSlot].pass);
      }
    }

    // 3. Auto Wi-Fi Power-Off Policy
    if (_wifiEnabled) {
      bool apHasClient = (WiFi.softAPgetStationNum() > 0);
      bool recentWebUse = (now - _lastWebActivityMs < WIFI_SEARCH_TIMEOUT_MS);
      uint32_t onDuration = now - _wifiTurnedOnMs;

      if (inGame && !apHasClient && (now - _lastWebActivityMs > 15000)) {
        // Entered a game and nobody is using the web portal:
        // If time is already synced OR we already tried for 20s, turn off Wi-Fi to save power & boost FPS
        if (isTimeSynced() || onDuration > 20000) {
          requestWifiOff();
        }
      } else if (!apHasClient && !recentWebUse && onDuration >= WIFI_SEARCH_TIMEOUT_MS) {
        // Searching / idle in menu or screensaver for >45s with no web activity:
        // If Wi-Fi couldn't connect, or if time is synced and nobody opened the web page, turn off Wi-Fi!
        if (WiFi.status() != WL_CONNECTED || inIdle) {
          requestWifiOff();
        }
      }
    }

    // 4. Dynamic CPU Frequency Scaling
    if (_wifiEnabled || inGame) {
      setCpuSpeed(160);
    } else {
      // Menu or Idle with Wi-Fi OFF -> 80 MHz saves ~35% CPU power
      setCpuSpeed(80);
    }

    // 5. Progressive Backlight Dimming
    uint8_t targetBri = gSettings.data.brightness;
    if (inIdle) {
      if (idleSec >= 300) {
        targetBri = min((uint8_t)10, targetBri); // Deep Eco Dim (10%) after 5 mins
      } else {
        targetBri = min((uint8_t)38, targetBri); // Idle Dim (38%) after 1 min
      }
    }
    if (targetBri != _appliedBrightness) {
      _appliedBrightness = targetBri;
      display.setBrightness(_appliedBrightness);
    }
  }
};

extern PowerManager gPower;
