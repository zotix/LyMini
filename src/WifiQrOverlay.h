#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "qrcode.h"
#include "DisplayCanvas.h"
#include "Config.h"

class WifiQrOverlay {
private:
  uint32_t _untilMs = 0;
  char _url[48] = "";
  char _ssid[33] = "";
  char _ipStr[24] = "";
  bool _active = false;
  bool _prevStaConnected = false;
  bool _firstBootApShown = false;

  static DisplayCanvas* s_disp;

  static void qrRenderCallback(esp_qrcode_handle_t qrcode) {
    if (!s_disp) return;
    int size = esp_qrcode_get_size(qrcode);
    if (size <= 0) return;

    // Calculate scale to fit nicely in 128x128 screen
    // Version 2 (25) * 3 = 75px, Version 3 (29) * 3 = 87px
    int scale = 87 / size;
    if (scale < 2) scale = 2;
    int totalPx = size * scale;
    int startX = (SCREEN_WIDTH - totalPx) / 2;
    int startY = 16 + (94 - totalPx) / 2;

    // White quiet border
    s_disp->fillRect(startX - 3, startY - 3, totalPx + 6, totalPx + 6, Color565::White);

    for (int y = 0; y < size; ++y) {
      for (int x = 0; x < size; ++x) {
        if (esp_qrcode_get_module(qrcode, x, y)) {
          s_disp->fillRect(startX + x * scale, startY + y * scale, scale, scale, Color565::Black);
        }
      }
    }
  }

public:
  void trigger(const char* url, const char* ssid, const char* ip, uint32_t durationMs = 5000) {
    strncpy(_url, url, sizeof(_url) - 1);
    strncpy(_ssid, ssid, sizeof(_ssid) - 1);
    strncpy(_ipStr, ip, sizeof(_ipStr) - 1);
    _untilMs = millis() + durationMs;
    _active = true;
  }

  void dismiss() {
    _active = false;
    _untilMs = 0;
  }

  bool isActive() const {
    return _active && (millis() < _untilMs);
  }

  uint8_t getRemainingSeconds() const {
    if (!isActive()) return 0;
    uint32_t rem = _untilMs - millis();
    return (uint8_t)((rem + 999) / 1000);
  }

  void checkWifiStatus() {
    bool staConnected = (WiFi.status() == WL_CONNECTED);
    if (staConnected && !_prevStaConnected) {
      // STA connected successfully!
      IPAddress ip = WiFi.localIP();
      char url[48];
      snprintf(url, sizeof(url), "http://%s", ip.toString().c_str());
      trigger(url, WiFi.SSID().c_str(), ip.toString().c_str(), 5000);
    }
    _prevStaConnected = staConnected;

    // Initial AP mode detection on first boot
    if (!_firstBootApShown && !staConnected && (WiFi.getMode() & WIFI_AP)) {
      IPAddress apIp = WiFi.softAPIP();
      if (apIp[0] != 0) {
        char url[48];
        snprintf(url, sizeof(url), "http://%s", apIp.toString().c_str());
        trigger(url, "LyMini-AP", apIp.toString().c_str(), 5000);
        _firstBootApShown = true;
      }
    }

    if (_active && millis() >= _untilMs) {
      _active = false;
    }
  }

  void render(DisplayCanvas& gfx) {
    if (!isActive()) return;

    // 1. Dark Navy background
    gfx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color565::rgb(10, 16, 32));

    // 2. Top Header Banner
    gfx.fillRect(0, 0, SCREEN_WIDTH, 14, Color565::rgb(0, 70, 140));
    gfx.drawCenteredText(3, "WIFI CONNECTED!", Color565::Gold);

    // 3. Render QR Code
    s_disp = &gfx;
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    cfg.display_func = qrRenderCallback;
    cfg.max_qrcode_version = 4;
    cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;
    esp_qrcode_generate(&cfg, _url);

    // 4. Bottom Info: IP + Countdown seconds
    char btm[32];
    uint8_t sec = getRemainingSeconds();
    snprintf(btm, sizeof(btm), "%s (%us)", _ipStr, sec);
    gfx.fillRect(0, 114, SCREEN_WIDTH, 14, Color565::rgb(0, 30, 60));
    gfx.drawCenteredText(117, btm, Color565::NeonCyan);
  }
};

DisplayCanvas* WifiQrOverlay::s_disp = nullptr;
