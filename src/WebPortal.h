#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "SystemSettings.h"
#include "WebPortalHtml.h"
#include "HighScoreStore.h"
#include "PowerManager.h"
#include "DisplayCanvas.h"
#include "IdleScene.h"
#include "PromptPayQrAsset.h"

class WebPortal {
private:
  WebServer      _server;
  DNSServer      _dnsServer;
  bool           _dnsRunning = false;
  File           _uploadFile;
  String         _lastUploadedPath;
  uint32_t       _livePixelIdx = 0;
  bool           _requestEnterIdle = false;

  DisplayCanvas* _display = nullptr;
  IdleScene*     _idle    = nullptr;

  static inline uint16_t expandRgb332(uint8_t c) {
    uint8_t r3 = (c >> 5) & 0x07;
    uint8_t g3 = (c >> 2) & 0x07;
    uint8_t b2 = c & 0x03;
    uint16_t r5 = (r3 << 2) | (r3 >> 1);
    uint16_t g6 = (g3 << 3) | g3;
    uint16_t b5 = (b2 << 3) | (b2 << 1) | (b2 >> 1);
    return (r5 << 11) | (g6 << 5) | b5;
  }

  void appendFolderJson(String& json, const char* dirPath, bool& first) {
    File dir = LittleFS.open(dirPath);
    if (!dir || !dir.isDirectory()) return;
    File f = dir.openNextFile();
    while (f) {
      if (!f.isDirectory()) {
        String name = f.name();
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
        if (name.endsWith(".rgb") || name.endsWith(".mvid")) {
          if (!first) json += ",";
          first = false;
          String fullPath = String(dirPath) + "/" + name;
          bool en = gSettings.isMediaEnabled(name.c_str());
          uint32_t kb = (f.size() + 1023) / 1024;
          json += "{\"name\":\"" + name + "\",\"path\":\"" + fullPath + "\",\"kb\":" + String(kb) +
                  ",\"enabled\":" + (en ? "true" : "false") + "}";
        }
      }
      f = dir.openNextFile();
    }
  }

  void sendCaptiveRedirect() {
    gPower.notifyWebActivity();
    _server.sendHeader("Location", "http://192.168.4.1/", true);
    _server.send(302, "text/plain", "Redirecting to LyMini Portal...");
  }

public:
  WebPortal() : _server(80) {}

  bool consumeIdleViewRequest() {
    if (_requestEnterIdle) {
      _requestEnterIdle = false;
      return true;
    }
    return false;
  }

  void begin(DisplayCanvas* display, IdleScene* idle) {
    _display = display;
    _idle    = idle;

    LittleFS.begin(true);
    if (!LittleFS.exists("/photos")) LittleFS.mkdir("/photos");
    if (!LittleFS.exists("/videos")) LittleFS.mkdir("/videos");

    // 1. Main Web Portal UI + OS Captive Portal Auto-Popup Endpoints
    _server.on("/", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      _server.send_P(200, "text/html", PAGE_INDEX_HTML);
    });

    _server.on("/hotspot-detect.html", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/library/test/success.html", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/generate_204", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/gen_204", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/ncsi.txt", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/connecttest.txt", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/redirect", HTTP_GET, [this]() { sendCaptiveRedirect(); });
    _server.on("/canonical.html", HTTP_GET, [this]() { sendCaptiveRedirect(); });

    _server.onNotFound([this]() {
      gPower.notifyWebActivity();
      _server.sendHeader("Location", "http://192.168.4.1/", true);
      _server.send(302, "text/plain", "");
    });

    // 2. Heartbeat Ping Endpoint (Keeps Wi-Fi alive while Web UI is open & shows Online status)
    _server.on("/api/ping", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      String json = "{\"ok\":true,\"batt\":" + String(gPower.getBatteryPercent()) +
                    ",\"usb\":" + String(gPower.isUsbOrCharging() ? "true" : "false") +
                    ",\"bright\":" + String(gSettings.data.brightness) + "}";
      _server.send(200, "application/json", json);
    });

    // 3. Instant Auto-Update Max Brightness Ceiling Endpoint
    _server.on("/api/brightness", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("val")) {
        uint8_t val = constrain(_server.arg("val").toInt(), 5, 100);
        gPower.applyMaxBrightnessNow(val, _display);
        if (_server.hasArg("save") && _server.arg("save").toInt() == 1) {
          gSettings.save();
        }
      }
      _server.send(200, "text/plain", "OK");
    });

    // 4. Preview Any Idle Screen Immediately on ESP32 Display
    _server.on("/api/preview_idle", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_idle && _server.hasArg("scr")) {
        uint8_t scr = (uint8_t)_server.arg("scr").toInt();
        int8_t cityIdx = _server.hasArg("cityIdx") ? (int8_t)_server.arg("cityIdx").toInt() : -1;
        _idle->previewScreenFromWeb(scr, cityIdx);
        _requestEnterIdle = true;
      }
      _server.send(200, "text/plain", "OK");
    });

    // 5. Serve PromptPay QR Code raw RGB332 (112x112)
    _server.on("/api/qr.bin", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      _server.send_P(200, "application/octet-stream", (const char*)PROMPTPAY_QR_332, sizeof(PROMPTPAY_QR_332));
    });

    _server.on("/api/show_qr", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_idle) {
        _idle->showPromptPayQrNow();
        _requestEnterIdle = true;
      }
      _server.send(200, "text/plain", "OK");
    });

    // 6. System Status + Saved Wi-Fi + Multi-City Weather + Media Gallery JSON
    _server.on("/api/status", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      String ip = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
      uint32_t totalKB = LittleFS.totalBytes() / 1024;
      uint32_t usedKB  = LittleFS.usedBytes() / 1024;

      String json = "{";
      json += "\"ip\":\"" + ip + "\",";
      json += "\"batt\":" + String(gPower.getBatteryPercent()) + ",";
      json += "\"usb\":" + String(gPower.isUsbOrCharging() ? "true" : "false") + ",";
      json += "\"timeSynced\":" + String(gPower.isTimeSynced() ? "true" : "false") + ",";
      json += "\"weatherValid\":" + String(gPower.hasValidWeather() ? "true" : "false") + ",";
      json += "\"fsTotalKB\":" + String(totalKB) + ",";
      json += "\"fsUsedKB\":" + String(usedKB) + ",";
      json += "\"idleMask\":" + String(gSettings.data.idle_enabled_mask | IDLE_MASK_ABOUT_QR) + ",";
      json += "\"idleRandom\":" + String(gSettings.data.idle_random_mode) + ",";
      json += "\"idleLockQr\":" + String(gSettings.data.idle_lock_qr) + ",";
      json += "\"idleTimeout\":" + String(gSettings.data.idle_timeout_sec) + ",";
      json += "\"idleCycle\":" + String(gSettings.data.idle_cycle_sec) + ",";
      json += "\"bright\":" + String(gSettings.data.brightness) + ",";
      json += "\"rot\":" + String(gSettings.data.screen_rotation) + ",";
      json += "\"btnRot\":" + String(gSettings.data.btn_rotate_with_screen) + ",";
      json += "\"bgr\":" + String(gSettings.data.color_order_bgr) + ",";
      json += "\"nightEn\":" + String(gSettings.data.night_mode_enabled) + ",";
      json += "\"nightStart\":" + String(gSettings.data.night_start_hour) + ",";
      json += "\"nightEnd\":" + String(gSettings.data.night_end_hour) + ",";

      json += "\"cities\":[";
      for (int i = 0; i < gSettings.data.weather_count; ++i) {
        if (i > 0) json += ",";
        json += "{\"name\":\"" + String(gSettings.data.weather_locs[i].name) + "\","
                "\"valid\":" + String(gPower.hasValidWeather(i) ? "true" : "false") + ","
                "\"temp\":" + String(gPower.getWeatherTempC(i), 1) + ","
                "\"hum\":" + String(gPower.getWeatherHumidity(i)) + "}";
      }
      json += "],\"wifiList\":[";
      for (int i = 0; i < gSettings.data.wifi_count; ++i) {
        if (i > 0) json += ",";
        json += "\"" + String(gSettings.data.wifi_list[i].ssid) + "\"";
      }
      json += "],\"media\":[";
      bool first = true;
      appendFolderJson(json, "/photos", first);
      appendFolderJson(json, "/videos", first);
      json += "]}";

      _server.send(200, "application/json", json);
    });

    // 7. Real-Time Live Frame Streaming to ESP32 Screen (/api/liveframe)
    _server.on("/api/liveframe", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_display && _idle) {
        _idle->activateLiveViewBuffer();
        _requestEnterIdle = true;
        _display->present();
      }
      _server.send(200, "text/plain", "OK");
    }, [this]() {
      HTTPUpload& up = _server.upload();
      if (!_display) return;
      uint16_t* dst = _display->getBuffer();
      if (up.status == UPLOAD_FILE_START) {
        gPower.notifyWebActivity();
        _livePixelIdx = 0;
      } else if (up.status == UPLOAD_FILE_WRITE) {
        for (size_t i = 0; i < up.currentSize && _livePixelIdx < (SCREEN_WIDTH * SCREEN_HEIGHT); ++i) {
          dst[_livePixelIdx++] = expandRgb332(up.buf[i]);
        }
      }
    });

    // 8. Upload Photo (.rgb) or Video (.mvid) and immediately display/play on ESP32
    _server.on("/api/upload", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_idle && _lastUploadedPath.length() > 0) {
        _idle->clearLiveViewBuffer();
        _idle->showSpecificMedia(_lastUploadedPath.c_str());
        _requestEnterIdle = true;
      }
      _server.send(200, "text/plain", "Upload OK");
    }, [this]() {
      HTTPUpload& upload = _server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        gPower.notifyWebActivity();
        if (_idle) _idle->clearLiveViewBuffer();
        String fname = upload.filename;
        int slash = fname.lastIndexOf('/');
        if (slash >= 0) fname = fname.substring(slash + 1);
        String folder = _server.hasArg("folder") ? _server.arg("folder") : "";
        if (folder == "videos" || fname.endsWith(".mvid")) {
          if (!fname.endsWith(".mvid")) fname += ".mvid";
          _lastUploadedPath = "/videos/" + fname;
        } else {
          if (!fname.endsWith(".rgb")) fname += ".rgb";
          _lastUploadedPath = "/photos/" + fname;
        }
        _uploadFile = LittleFS.open(_lastUploadedPath, "w");
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (_uploadFile) _uploadFile.write(upload.buf, upload.currentSize);
      } else if (upload.status == UPLOAD_FILE_END) {
        if (_uploadFile) _uploadFile.close();
      }
    });

    // 9. Show specific file immediately on ESP32 screen
    _server.on("/api/show", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("path") && _idle) {
        _idle->clearLiveViewBuffer();
        _idle->showSpecificMedia(_server.arg("path").c_str());
        _requestEnterIdle = true;
      }
      _server.send(200, "text/plain", "OK");
    });

    // 10. Enable/Disable media file in Idle rotation
    _server.on("/api/toggle_media", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("name")) {
        bool en = (_server.arg("en").toInt() != 0);
        gSettings.setMediaEnabled(_server.arg("name").c_str(), en);
        if (_idle) _idle->scanMediaFiles();
      }
      _server.send(200, "text/plain", "OK");
    });

    // 11. Delete media file
    _server.on("/api/delete_media", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("path")) {
        String p = _server.arg("path");
        if (LittleFS.exists(p)) LittleFS.remove(p);
        if (_idle) _idle->scanMediaFiles();
      }
      _server.send(200, "text/plain", "OK");
    });

    // 12. Scan nearby Wi-Fi networks
    _server.on("/api/scan_wifi", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      int n = WiFi.scanNetworks();
      String json = "[";
      for (int i = 0; i < n && i < 12; ++i) {
        if (i > 0) json += ",";
        String ssid = WiFi.SSID(i);
        ssid.replace("\"", "");
        json += "{\"ssid\":\"" + ssid + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
      }
      json += "]";
      WiFi.scanDelete();
      _server.send(200, "application/json", json);
    });

    // 13. Save Wi-Fi credential persistently in NVS
    _server.on("/api/save_wifi", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("ssid")) {
        String s = _server.arg("ssid");
        String p = _server.arg("pass");
        gSettings.addOrUpdateWifi(s.c_str(), p.c_str());
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(s.c_str(), p.c_str());
      }
      _server.send(200, "text/plain", "OK");
    });

    // 14. Delete saved Wi-Fi credential
    _server.on("/api/del_wifi", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("idx")) {
        gSettings.removeWifi(_server.arg("idx").toInt());
      }
      _server.send(200, "text/plain", "OK");
    });

    // 15. Add / Delete / Refresh Multi-Location Weather Cities
    _server.on("/api/add_city", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("city")) {
        String c = _server.arg("city");
        c.trim();
        if (c.length() > 0) {
          gPower.geocodeAndAddCity(c.c_str());
        }
      }
      _server.send(200, "text/plain", "OK");
    });

    _server.on("/api/del_city", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("idx")) {
        gSettings.removeWeatherCity(_server.arg("idx").toInt());
      }
      _server.send(200, "text/plain", "OK");
    });

    _server.on("/api/refresh_weather", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.requestWeatherRefresh();
      _server.send(200, "text/plain", "OK");
    });

    // 16. Save Idle, Display, Rotation & Night Mode settings
    _server.on("/api/settings", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("bright")) {
        uint8_t b = constrain(_server.arg("bright").toInt(), 5, 100);
        gPower.applyMaxBrightnessNow(b, _display);
      }
      if (_server.hasArg("rot")) {
        gSettings.data.screen_rotation = _server.arg("rot").toInt() & 3;
      }
      if (_server.hasArg("btnrot")) {
        gSettings.data.btn_rotate_with_screen = (_server.arg("btnrot").toInt() != 0) ? 1 : 0;
      }
      if (_server.hasArg("bgr")) {
        gSettings.data.color_order_bgr = (_server.arg("bgr").toInt() != 0) ? 1 : 0;
      }
      if (_display) {
        _display->setRotation(gSettings.data.screen_rotation, gSettings.data.color_order_bgr != 0);
      }
      if (_server.hasArg("night_en")) {
        gSettings.data.night_mode_enabled = (_server.arg("night_en").toInt() != 0) ? 1 : 0;
      }
      if (_server.hasArg("night_st")) {
        gSettings.data.night_start_hour = _server.arg("night_st").toInt() % 24;
      }
      if (_server.hasArg("night_ed")) {
        gSettings.data.night_end_hour = _server.arg("night_ed").toInt() % 24;
      }
      if (_server.hasArg("idletime")) {
        gSettings.data.idle_timeout_sec = constrain(_server.arg("idletime").toInt(), 15, 600);
      }
      if (_server.hasArg("idlecyc")) {
        gSettings.data.idle_cycle_sec = constrain(_server.arg("idlecyc").toInt(), 5, 120);
      }
      if (_server.hasArg("idlemask")) {
        // PromptPay QR (IDLE_MASK_ABOUT_QR = 0x40) is mandatory and always forced ON!
        gSettings.data.idle_enabled_mask = ((uint8_t)_server.arg("idlemask").toInt()) | IDLE_MASK_ABOUT_QR;
      }
      if (_server.hasArg("idlernd")) {
        gSettings.data.idle_random_mode = (uint8_t)_server.arg("idlernd").toInt();
      }
      if (_server.hasArg("lockqr")) {
        gSettings.data.idle_lock_qr = (_server.arg("lockqr").toInt() != 0) ? 1 : 0;
      }
      gSettings.save();
      if (_idle) _idle->scanMediaFiles();
      _server.send(200, "text/plain", "Settings Saved");
    });

    _server.on("/api/resetscores", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      HighScoreStore::resetAllScores();
      _server.send(200, "text/plain", "Scores Reset");
    });

    _server.begin();
  }

  void update() {
    if (gPower.isWifiEnabled()) {
      if (!_dnsRunning) {
        _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        _dnsServer.start(53, "*", WiFi.softAPIP());
        _dnsRunning = true;
      }
      _dnsServer.processNextRequest();
      _server.handleClient();
    } else if (_dnsRunning) {
      _dnsServer.stop();
      _dnsRunning = false;
    }
  }
};
