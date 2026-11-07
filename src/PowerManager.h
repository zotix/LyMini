#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
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
  uint8_t  _previewBrightnessVal = 85;
  uint32_t _brightnessPreviewUntilMs = 0; // Holds previewed brightness while adjusting via Web or Menu

  // Multi-location Weather state (fetched automatically when Wi-Fi STA connects)
  bool     _locValid[MAX_WEATHER_CITIES] = {false};
  float    _locTempC[MAX_WEATHER_CITIES] = {0.0f};
  uint8_t  _locHumid[MAX_WEATHER_CITIES] = {0};
  uint32_t _lastWeatherFetchMs = 0;

  void detectExternalAdc() {
    pinMode(1, INPUT_PULLUP);
    delay(2);
    int vHigh = analogRead(1);
    pinMode(1, INPUT_PULLDOWN);
    delay(2);
    int vLow = analogRead(1);
    pinMode(1, INPUT);
    if (abs(vHigh - vLow) < 600 && vLow > 800 && vHigh < 3600) {
      _hasExternalAdc = true;
    } else {
      _hasExternalAdc = false;
    }
  }

  void setCpuSpeed(uint8_t mhz) {
    if (_currentCpuMhz == mhz) return;
    if (_wifiEnabled && mhz < 160) mhz = 160;
    setCpuFrequencyMhz(mhz);
    _currentCpuMhz = mhz;
  }

public:
  void begin() {
    if (gRtcPwrMagic != RTC_PWR_MAGIC) {
      gRtcPwrMagic = RTC_PWR_MAGIC;
      gRtcBattPermille = 960;
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

  // Apply and preview Max Brightness immediately (even if device is currently in Idle mode!)
  void applyMaxBrightnessNow(uint8_t maxBri, DisplayCanvas* display) {
    gSettings.data.brightness = constrain(maxBri, 5, 100);
    _previewBrightnessVal = gSettings.data.brightness;
    _brightnessPreviewUntilMs = millis() + 15000UL;
    _appliedBrightness = _previewBrightnessVal;
    if (display) {
      display->setBrightness(_appliedBrightness);
    }
  }

  // Apply and preview Eco / Power-Saving Brightness immediately on physical display
  void applyEcoBrightnessNow(uint8_t ecoBri, DisplayCanvas* display) {
    gSettings.data.eco_brightness = constrain(ecoBri, 5, 100);
    uint8_t maxBri = constrain(gSettings.data.brightness, 5, 100);
    _previewBrightnessVal = min(gSettings.data.eco_brightness, maxBri);
    _brightnessPreviewUntilMs = millis() + 12000UL;
    _appliedBrightness = _previewBrightnessVal;
    if (display) {
      display->setBrightness(_appliedBrightness);
    }
  }

  // Force-disable Night Mode Screen-Off and wake screen to Max Brightness so Live View is always visible
  void wakeForLiveView(DisplayCanvas* display) {
    if (gSettings.data.night_mode_enabled != 0) {
      gSettings.data.night_mode_enabled = 0;
      gSettings.save();
    }
    _previewBrightnessVal = constrain(gSettings.data.brightness, 5, 100);
    _brightnessPreviewUntilMs = millis() + 30000UL;
    _appliedBrightness = _previewBrightnessVal;
    if (display) {
      display->setBrightness(_appliedBrightness);
    }
  }

  bool hasValidWeather(int idx = -1) const {
    if (idx >= 0 && idx < gSettings.data.weather_count) {
      return _locValid[idx];
    }
    for (int i = 0; i < gSettings.data.weather_count; ++i) {
      if (_locValid[i]) return true;
    }
    return false;
  }

  float getWeatherTempC(int idx = 0) const {
    if (idx < 0 || idx >= gSettings.data.weather_count) idx = 0;
    return _locTempC[idx];
  }

  uint8_t getWeatherHumidity(int idx = 0) const {
    if (idx < 0 || idx >= gSettings.data.weather_count) idx = 0;
    return _locHumid[idx];
  }

  const char* getWeatherCityName(int idx = 0) const {
    if (idx < 0 || idx >= gSettings.data.weather_count) idx = 0;
    return gSettings.data.weather_locs[idx].name;
  }

  // Geocode a city name via Open-Meteo and add/update it in SystemSettings
  bool geocodeAndAddCity(const char* cityName) {
    if (!cityName || strlen(cityName) == 0) return false;
    float lat = 13.7563f, lon = 100.5018f;

    if (_wifiEnabled && WiFi.status() == WL_CONNECTED) {
      HTTPClient geo;
      String encodedCity = String(cityName);
      encodedCity.trim();
      encodedCity.replace(" ", "%20");
      String geoUrl = "http://geocoding-api.open-meteo.com/v1/search?name=" + encodedCity + "&count=1&language=en&format=json";
      geo.setTimeout(2500);
      geo.begin(geoUrl);
      if (geo.GET() == HTTP_CODE_OK) {
        String p = geo.getString();
        int latIdx = p.indexOf("\"latitude\":");
        int lonIdx = p.indexOf("\"longitude\":");
        if (latIdx > 0 && lonIdx > 0) {
          lat = p.substring(latIdx + 11).toFloat();
          lon = p.substring(lonIdx + 12).toFloat();
        }
      }
      geo.end();
    }

    int slot = gSettings.addOrUpdateWeatherCity(cityName, lat, lon);
    if (slot >= 0 && _wifiEnabled && WiFi.status() == WL_CONNECTED) {
      fetchSingleLocationWeather(slot);
    } else {
      _lastWeatherFetchMs = 0;
    }
    return (slot >= 0);
  }

  void fetchSingleLocationWeather(int idx) {
    if (idx < 0 || idx >= gSettings.data.weather_count) return;
    if (!_wifiEnabled || WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    char url[160];
    snprintf(url, sizeof(url),
             "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m",
             gSettings.data.weather_locs[idx].lat, gSettings.data.weather_locs[idx].lon);
    http.setTimeout(2500);
    http.begin(url);
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      int curIdx = payload.indexOf("\"current\":");
      int searchFrom = (curIdx > 0) ? curIdx : 0;
      int tempIdx = payload.indexOf("\"temperature_2m\":", searchFrom);
      int humIdx  = payload.indexOf("\"relative_humidity_2m\":", searchFrom);
      if (tempIdx > 0) {
        _locTempC[idx] = payload.substring(tempIdx + 17).toFloat();
        if (humIdx > 0) _locHumid[idx] = payload.substring(humIdx + 23).toInt();
        _locValid[idx] = true;
      }
    }
    http.end();
  }

  void fetchAllWeatherNow() {
    if (!_wifiEnabled || WiFi.status() != WL_CONNECTED) return;
    _lastWeatherFetchMs = millis();
    for (int i = 0; i < gSettings.data.weather_count; ++i) {
      fetchSingleLocationWeather(i);
    }
  }

  void requestWeatherRefresh() {
    _lastWeatherFetchMs = 0;
    if (!_wifiEnabled) {
      requestWifiOn();
    } else if (WiFi.status() == WL_CONNECTED) {
      fetchAllWeatherNow();
    }
  }

  bool isNightScreenOffTime() const {
    if (!gSettings.data.night_mode_enabled) return false;
    if (!isTimeSynced()) return false;
    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);
    uint8_t h = (uint8_t)ti.tm_hour;
    uint8_t st = gSettings.data.night_start_hour % 24;
    uint8_t ed = gSettings.data.night_end_hour % 24;
    if (st == ed) return false;
    if (st < ed) {
      return (h >= st && h < ed);
    } else {
      return (h >= st || h < ed);
    }
  }

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

    // Open SoftAP "LyMini-AP" with NO password required
    WiFi.softAP("LyMini-AP");

    if (!_mdnsStarted) {
      if (MDNS.begin("lymini")) {
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
      uint32_t mv = analogReadMilliVolts(1) * 2;
      if (mv >= 4150) _batteryPct = 100;
      else if (mv <= 3300) _batteryPct = 5;
      else _batteryPct = (uint8_t)(((mv - 3300) * 95) / 850 + 5);
      gRtcBattPermille = _batteryPct * 10;
    } else {
      if (_usbConnected) {
        if (gRtcBattPermille < 1000) gRtcBattPermille += 2;
        if (gRtcBattPermille > 1000) gRtcBattPermille = 1000;
      } else {
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

    // 2.5 Auto Weather Fetch as soon as Wi-Fi STA connects (every 10 mins)
    if (_wifiEnabled && WiFi.status() == WL_CONNECTED) {
      if (!hasValidWeather() || _lastWeatherFetchMs == 0 || (now - _lastWeatherFetchMs >= 600000UL)) {
        if (_lastWeatherFetchMs == 0 || (now - _lastWeatherFetchMs >= 15000UL)) {
          fetchAllWeatherNow();
        }
      }
    }

    // 3. Auto Wi-Fi Power-Off Policy (Never turns off while Web UI is open & pinging!)
    if (_wifiEnabled) {
      bool apHasClient = (WiFi.softAPgetStationNum() > 0);
      bool recentWebUse = (now - _lastWebActivityMs < WIFI_SEARCH_TIMEOUT_MS);
      uint32_t onDuration = now - _wifiTurnedOnMs;

      if (inGame && !apHasClient && (now - _lastWebActivityMs > 15000)) {
        if ((isTimeSynced() && hasValidWeather()) || onDuration > 25000) {
          requestWifiOff();
        }
      } else if (!apHasClient && !recentWebUse && onDuration >= WIFI_SEARCH_TIMEOUT_MS) {
        if (WiFi.status() != WL_CONNECTED || inIdle) {
          requestWifiOff();
        }
      }
    }

    // 4. Dynamic CPU Frequency Scaling
    if (_wifiEnabled || inGame) {
      setCpuSpeed(160);
    } else {
      setCpuSpeed(80);
    }

    // 5. Max Brightness Ceiling + Configurable Eco/Idle Brightness + Night Mode Schedule
    uint8_t maxBri = constrain(gSettings.data.brightness, 5, 100);
    uint8_t ecoBri = min((uint8_t)constrain(gSettings.data.eco_brightness, 5, 100), maxBri);
    uint8_t targetBri = maxBri;

    if (now <= _brightnessPreviewUntilMs) {
      targetBri = min(_previewBrightnessVal, maxBri);
    } else if (inIdle) {
      if (isNightScreenOffTime()) {
        targetBri = 0; // Night Mode Screen-Off (0%)
      } else {
        targetBri = ecoBri; // Configurable Idle / Eco Power-Saving Brightness
      }
    }

    // Strictly guarantee targetBri never exceeds maxBri
    if (targetBri > maxBri) targetBri = maxBri;

    if (targetBri != _appliedBrightness) {
      _appliedBrightness = targetBri;
      display.setBrightness(_appliedBrightness);
    }
  }
};

extern PowerManager gPower;
