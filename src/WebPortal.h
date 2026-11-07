#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "SystemSettings.h"
#include "WebPortalHtml.h"
#include "HighScoreStore.h"

class WebPortal {
private:
  WebServer _server;
  File _uploadFile;
  bool _apMode;

public:
  WebPortal() : _server(80), _apMode(false) {}

  void begin() {
    // Setup LittleFS for storage
    LittleFS.begin(true);
    if (!LittleFS.exists("/photos")) {
      LittleFS.mkdir("/photos");
    }

    // Connect to Wi-Fi if configured
    if (strlen(gSettings.data.wifi_ssid) > 0) {
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(gSettings.data.wifi_ssid, gSettings.data.wifi_pass);
    } else {
      WiFi.mode(WIFI_AP);
    }

    // Always keep fallback SoftAP active so user is never locked out
    WiFi.softAP("Spotpear-Mini", "12345678");

    // Start mDNS Responder for http://mini.local
    if (MDNS.begin("mini")) {
      MDNS.addService("http", "tcp", 80);
    }

    // Setup Web Routes
    _server.on("/", HTTP_GET, [this]() {
      _server.send_P(200, "text/html", PAGE_INDEX_HTML);
    });

    _server.on("/api/settings", HTTP_POST, [this]() {
      if (_server.hasArg("ssid")) {
        strncpy(gSettings.data.wifi_ssid, _server.arg("ssid").c_str(), sizeof(gSettings.data.wifi_ssid) - 1);
      }
      if (_server.hasArg("pass")) {
        strncpy(gSettings.data.wifi_pass, _server.arg("pass").c_str(), sizeof(gSettings.data.wifi_pass) - 1);
      }
      if (_server.hasArg("city")) {
        strncpy(gSettings.data.city_name, _server.arg("city").c_str(), sizeof(gSettings.data.city_name) - 1);
      }
      if (_server.hasArg("bright")) {
        gSettings.data.brightness = _server.arg("bright").toInt();
      }
      if (_server.hasArg("idletime")) {
        gSettings.data.idle_timeout_sec = _server.arg("idletime").toInt();
      }
      gSettings.save();
      _server.send(200, "text/html", "<h2>Settings Saved! <a href='/'>Go Back</a></h2>");
    });

    _server.on("/api/resetscores", HTTP_POST, [this]() {
      HighScoreStore::resetAllScores();
      _server.send(200, "text/html", "<h2>Scores Reset! <a href='/'>Go Back</a></h2>");
    });

    // File Upload Handler (Raw 128x128 RGB565 images or media files)
    _server.on("/api/upload", HTTP_POST, [this]() {
      _server.send(200, "text/plain", "Upload OK");
    }, [this]() {
      HTTPUpload& upload = _server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        String filename = "/photos/" + upload.filename;
        if (!filename.endsWith(".rgb")) filename += ".rgb";
        _uploadFile = LittleFS.open(filename, "w");
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (_uploadFile) {
          _uploadFile.write(upload.buf, upload.currentSize);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (_uploadFile) {
          _uploadFile.close();
        }
      }
    });

    _server.begin();
  }

  void update() {
    _server.handleClient();
  }
};
