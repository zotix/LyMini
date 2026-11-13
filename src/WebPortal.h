#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Update.h>
#include "SystemSettings.h"
#include "WebPortalAssets.h"
#include "HighScoreStore.h"
#include "PowerManager.h"
#include "DisplayCanvas.h"
#include "IdleScene.h"
#include "GameRegistry.h"
#include "PromptPayQrAsset.h"
#include "OtaUpdater.h"

class WebPortal {
private:
  WebServer      _server;
  DNSServer      _dnsServer;
  bool           _dnsRunning = false;
  File           _uploadFile;
  String         _lastUploadedPath;
  uint32_t       _livePixelIdx = 0;
  uint8_t        _uploadVidFormat = 0; // 0 = HD 16-bit RGB565 (default), 1 = Compact 8-bit RGB332
  bool           _requestEnterIdle = false;

  DisplayCanvas* _display  = nullptr;
  IdleScene*     _idle     = nullptr;
  GameRegistry*  _registry = nullptr;

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

  void ensureFreeSpaceForMedia(size_t neededBytes, const char* preferredDir) {
    if (_idle) _idle->closeActiveVideo();
    for (int pass = 0; pass < 8; ++pass) {
      size_t totalB = LittleFS.totalBytes();
      size_t usedB  = LittleFS.usedBytes();
      if (totalB > usedB && (totalB - usedB) >= neededBytes) return;

      String victim = "";
      const char* dirs[2] = { preferredDir, "/videos" };
      for (int d = 0; d < 2 && victim.length() == 0; ++d) {
        File dir = LittleFS.open(dirs[d]);
        if (dir && dir.isDirectory()) {
          File f = dir.openNextFile();
          while (f) {
            if (!f.isDirectory()) {
              String n = f.name();
              int sl = n.lastIndexOf('/');
              if (sl >= 0) n = n.substring(sl + 1);
              victim = String(dirs[d]) + "/" + n;
              break;
            }
            f = dir.openNextFile();
          }
        }
      }
      if (victim.length() > 0) {
        LittleFS.remove(victim);
      } else {
        break;
      }
    }
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

  void begin(DisplayCanvas* display, IdleScene* idle, GameRegistry* registry = nullptr) {
    _display  = display;
    _idle     = idle;
    _registry = registry;

    LittleFS.begin(true);
    if (!LittleFS.exists("/photos")) LittleFS.mkdir("/photos");
    if (!LittleFS.exists("/videos")) LittleFS.mkdir("/videos");

    // 1. Main Web Portal UI + OS Captive Portal Auto-Popup Endpoints
    static const char* headerKeys[] = {"If-None-Match", "Range"};
    _server.collectHeaders(headerKeys, 2);
    _server.on("/", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      _server.sendHeader("Cache-Control", "public, max-age=3600, must-revalidate");
      _server.sendHeader("ETag", PAGE_INDEX_HTML_ETAG);
      if (_server.hasHeader("If-None-Match") && _server.header("If-None-Match") == PAGE_INDEX_HTML_ETAG) {
        _server.send(304, "text/html", "");
        return;
      }
      _server.sendHeader("Content-Encoding", "gzip");
      _server.send_P(200, "text/html", (const char*)PAGE_INDEX_HTML_GZ, sizeof(PAGE_INDEX_HTML_GZ));
    });

    _server.on("/manifest.webmanifest", HTTP_GET, [this]() {
      _server.sendHeader("Cache-Control", "public, max-age=604800");
      _server.send(200, "application/manifest+json",
        "{\"name\":\"LyMini Smart TV\",\"short_name\":\"LyMini\",\"start_url\":\"/\",\"display\":\"standalone\","
        "\"background_color\":\"#0b0f17\",\"theme_color\":\"#0b0f17\","
        "\"icons\":[{\"src\":\"/icon-192.png\",\"sizes\":\"192x192\",\"type\":\"image/png\"},"
        "{\"src\":\"/icon.svg\",\"sizes\":\"any\",\"type\":\"image/svg+xml\"}]}");
    });

    _server.on("/icon-192.png", HTTP_GET, [this]() {
      _server.sendHeader("Cache-Control", "public, max-age=604800");
      _server.send_P(200, "image/png", (const char*)ICON_192_PNG, sizeof(ICON_192_PNG));
    });

    _server.on("/icon.svg", HTTP_GET, [this]() {
      _server.sendHeader("Cache-Control", "public, max-age=604800");
      _server.sendHeader("Content-Encoding", "gzip");
      _server.send_P(200, "image/svg+xml", (const char*)ICON_SVG_GZ, sizeof(ICON_SVG_GZ));
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

    // 3b. Instant Auto-Update Eco Power-Saving Brightness Endpoint
    _server.on("/api/eco_brightness", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("val")) {
        uint8_t val = constrain(_server.arg("val").toInt(), 5, 100);
        gPower.applyEcoBrightnessNow(val, _display);
        if (_server.hasArg("save") && _server.arg("save").toInt() == 1) {
          gSettings.save();
        }
      }
      _server.send(200, "text/plain", "OK");
    });

    // 3c. Instant Auto-Update Idle Brightness Endpoint
    _server.on("/api/idle_brightness", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_server.hasArg("val")) {
        uint8_t val = constrain(_server.arg("val").toInt(), 5, 100);
        gPower.applyIdleBrightnessNow(val, _display);
        if (_server.hasArg("save") && _server.arg("save").toInt() == 1) {
          gSettings.save();
        }
      }
      _server.send(200, "text/plain", "OK");
    });

    // 4. Preview Any Idle Screen Immediately on ESP32 Display
    _server.on("/api/preview_idle", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
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
      gPower.wakeForLiveView(_display);
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
      json += "\"cpuMhz\":" + String(gPower.getCpuMhz()) + ",";
      json += "\"idleMask\":" + String(gSettings.data.idle_enabled_mask | IDLE_MASK_ABOUT_QR) + ",";
      json += "\"idleRandom\":" + String(gSettings.data.idle_random_mode) + ",";
      json += "\"ecoVideo\":" + String(gSettings.data.eco_video_enabled) + ",";
      json += "\"idleLockQr\":" + String(gSettings.data.idle_lock_qr) + ",";
      json += "\"idleTimeout\":" + String(gSettings.data.idle_timeout_sec) + ",";
      json += "\"ecoTimeout\":" + String(gSettings.data.eco_timeout_sec) + ",";
      json += "\"sleepTimeout\":" + String(gSettings.data.sleep_timeout_sec) + ",";
      json += "\"idleCycle\":" + String(gSettings.data.idle_cycle_sec) + ",";
      json += "\"bright\":" + String(gSettings.data.brightness) + ",";
      json += "\"idleBright\":" + String(gSettings.data.idle_brightness) + ",";
      json += "\"ecoBright\":" + String(gSettings.data.eco_brightness) + ",";
      json += "\"version\":\"" + String(APP_VERSION) + "\",";
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
      json += "],\"games\":[";
      if (_registry) {
        uint32_t gmask = gSettings.data.game_enabled_mask & ((1UL << TOTAL_GAMES) - 1);
        for (int i = 0; i < TOTAL_GAMES; ++i) {
          uint8_t rawIdx = gSettings.data.game_order[i];
          if (rawIdx >= TOTAL_GAMES) rawIdx = i;
          IGame* gm = _registry->getGame(rawIdx);
          if (i > 0) json += ",";
          json += "{\"id\":" + String(rawIdx) + ","
                  "\"title\":\"" + String(gm->title()) + "\","
                  "\"enabled\":" + String((gmask & (1UL << rawIdx)) ? "true" : "false") + ","
                  "\"score\":" + String(HighScoreStore::getHighScore(gm->id())) + "}";
        }
      }
      json += "]}";

      _server.send(200, "application/json", json);
    });

    // 6b. Stream raw LittleFS photo/video file for Web Media Gallery Preview (supports HTTP Range!)
    _server.on("/api/media_file", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      if (!_server.hasArg("path")) { _server.send(400, "text/plain", "Missing path"); return; }
      String path = _server.arg("path");
      if ((!path.startsWith("/photos/") && !path.startsWith("/videos/")) || path.indexOf("..") >= 0) {
        _server.send(403, "text/plain", "Forbidden");
        return;
      }
      File f = LittleFS.open(path, "r");
      if (!f || f.isDirectory()) {
        if (f) f.close();
        _server.send(404, "text/plain", "Not found");
        return;
      }
      size_t totalSize = f.size();
      _server.sendHeader("Cache-Control", "public, max-age=86400");
      _server.sendHeader("Accept-Ranges", "bytes");

      if (_server.hasHeader("Range")) {
        String rng = _server.header("Range");
        if (rng.startsWith("bytes=")) {
          rng = rng.substring(6);
          int dash = rng.indexOf('-');
          size_t startPos = (size_t)rng.substring(0, dash).toInt();
          size_t endPos = (dash >= 0 && dash + 1 < (int)rng.length())
                            ? (size_t)rng.substring(dash + 1).toInt()
                            : (totalSize > 0 ? totalSize - 1 : 0);
          if (totalSize == 0 || startPos >= totalSize) {
            f.close();
            _server.send(416, "text/plain", "Range Not Satisfiable");
            return;
          }
          if (endPos >= totalSize) endPos = totalSize - 1;
          size_t sendLen = endPos - startPos + 1;
          f.seek(startPos, SeekSet);
          char crBuf[48];
          snprintf(crBuf, sizeof(crBuf), "bytes %u-%u/%u", (unsigned)startPos, (unsigned)endPos, (unsigned)totalSize);
          _server.sendHeader("Content-Range", crBuf);
          _server.setContentLength(sendLen);
          _server.send(206, "application/octet-stream", "");
          WiFiClient client = _server.client();
          uint8_t buf[512];
          size_t rem = sendLen;
          while (rem > 0 && f.available() && client.connected()) {
            size_t toRead = (rem < sizeof(buf)) ? rem : sizeof(buf);
            size_t rd = f.read(buf, toRead);
            if (rd == 0) break;
            client.write(buf, rd);
            rem -= rd;
          }
          f.close();
          return;
        }
      }

      _server.streamFile(f, "application/octet-stream");
      f.close();
    });

    // 7. Real-Time Live Frame Streaming to ESP32 Screen (/api/liveframe)
    _server.on("/api/liveframe", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
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

    // 7b. Stop Studio Live View when switching away from Studio Tab
    _server.on("/api/stop_live", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      if (_idle) {
        _idle->clearLiveViewBuffer();
      }
      _server.send(200, "text/plain", "OK");
    });

    // 8. Upload Photo (.rgb) or Video (.mvid) and immediately display/play on ESP32
    _server.on("/api/upload", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
      if (_uploadFile) _uploadFile.close();
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
        if (_idle) {
          _idle->clearLiveViewBuffer();
          _idle->closeActiveVideo();
        }
        if (_uploadFile) _uploadFile.close();
        String fname = upload.filename;
        int slash = fname.lastIndexOf('/');
        if (slash >= 0) fname = fname.substring(slash + 1);
        String folder = _server.hasArg("folder") ? _server.arg("folder") : "";
        if (folder == "videos" || fname.endsWith(".mvid")) {
          if (!fname.endsWith(".mvid")) fname += ".mvid";
          _lastUploadedPath = "/videos/" + fname;
          ensureFreeSpaceForMedia(195000, "/videos");
        } else {
          if (!fname.endsWith(".rgb")) fname += ".rgb";
          _lastUploadedPath = "/photos/" + fname;
          ensureFreeSpaceForMedia(48000, "/photos");
        }
        _uploadFile = LittleFS.open(_lastUploadedPath, "w");
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (_uploadFile) _uploadFile.write(upload.buf, upload.currentSize);
      } else if (upload.status == UPLOAD_FILE_END) {
        if (_uploadFile) {
          _uploadFile.flush();
          _uploadFile.close();
        }
      }
    });

    // 8b. Reliable Frame-by-Frame Video Upload Pipeline (/api/upload_init -> /api/upload_frame -> /api/upload_finish)
    _server.on("/api/upload_init", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
      if (_uploadFile) _uploadFile.close();
      if (_idle) {
        _idle->clearLiveViewBuffer();
        _idle->closeActiveVideo();
      }
      String fname = _server.hasArg("name") ? _server.arg("name") : ("vid_" + String(millis() % 100000) + ".mvid");
      int slash = fname.lastIndexOf('/');
      if (slash >= 0) fname = fname.substring(slash + 1);
      if (!fname.endsWith(".mvid")) fname += ".mvid";
      _lastUploadedPath = "/videos/" + fname;

      uint8_t format = _server.hasArg("format") ? constrain(_server.arg("format").toInt(), 0, 1) : 0; // 0 = HD RGB565 default
      _uploadVidFormat = format;
      uint8_t frames = _server.hasArg("frames") ? constrain(_server.arg("frames").toInt(), 1, 24) : 10;
      uint8_t fps    = _server.hasArg("fps") ? constrain(_server.arg("fps").toInt(), 4, 24) : 9;

      size_t bytesPerFrame = (format == 0) ? (SCREEN_WIDTH * SCREEN_HEIGHT * 2UL) : (SCREEN_WIDTH * SCREEN_HEIGHT);
      size_t needed = (size_t)frames * bytesPerFrame + 28672UL;
      ensureFreeSpaceForMedia(needed, "/videos");

      _uploadFile = LittleFS.open(_lastUploadedPath, "w");
      if (!_uploadFile) {
        _server.send(500, "text/plain", "ERR_OPEN");
        return;
      }
      uint8_t hdr[8] = { 'M', 'V', '1', format, 128, 128, fps, frames };
      _uploadFile.write(hdr, 8);
      _uploadFile.flush();
      _server.send(200, "text/plain", "OK");
    });

    _server.on("/api/upload_frame", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
      if (_uploadFile) _uploadFile.flush();
      if (_display && _idle) {
        _idle->activateLiveViewBuffer();
        _requestEnterIdle = true;
        _display->present();
      }
      _server.send(200, "text/plain", "OK");
    }, [this]() {
      HTTPUpload& up = _server.upload();
      if (up.status == UPLOAD_FILE_START) {
        gPower.notifyWebActivity();
        _livePixelIdx = 0;
      } else if (up.status == UPLOAD_FILE_WRITE) {
        if (_uploadFile) {
          _uploadFile.write(up.buf, up.currentSize);
        }
        if (_display) {
          uint16_t* dst = _display->getBuffer();
          if (_uploadVidFormat == 0) {
            for (size_t i = 0; i + 1 < up.currentSize && _livePixelIdx < (SCREEN_WIDTH * SCREEN_HEIGHT); i += 2) {
              dst[_livePixelIdx++] = ((uint16_t)up.buf[i] << 8) | up.buf[i + 1];
            }
          } else {
            for (size_t i = 0; i < up.currentSize && _livePixelIdx < (SCREEN_WIDTH * SCREEN_HEIGHT); ++i) {
              dst[_livePixelIdx++] = expandRgb332(up.buf[i]);
            }
          }
        }
      }
    });

    _server.on("/api/upload_finish", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
      if (_uploadFile) {
        _uploadFile.flush();
        _uploadFile.close();
      }
      if (_idle && _lastUploadedPath.length() > 0) {
        _idle->clearLiveViewBuffer();
        _idle->showSpecificMedia(_lastUploadedPath.c_str());
        _requestEnterIdle = true;
      }
      _server.send(200, "text/plain", "OK");
    });

    // 9. Show specific file immediately on ESP32 screen
    _server.on("/api/show", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      gPower.wakeForLiveView(_display);
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
      if (_server.hasArg("idlebri")) {
        gSettings.data.idle_brightness = constrain(_server.arg("idlebri").toInt(), 5, 100);
      }
      if (_server.hasArg("ecobri")) {
        gSettings.data.eco_brightness = constrain(_server.arg("ecobri").toInt(), 5, 100);
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
        gSettings.data.idle_timeout_sec = constrain(_server.arg("idletime").toInt(), 5, 600);
      }
      if (_server.hasArg("ecotime")) {
        gSettings.data.eco_timeout_sec = constrain(_server.arg("ecotime").toInt(), 30, 3600);
      }
      if (_server.hasArg("sleeptime")) {
        gSettings.data.sleep_timeout_sec = constrain(_server.arg("sleeptime").toInt(), 0, 86400);
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
      if (_server.hasArg("ecovid")) {
        gSettings.data.eco_video_enabled = (_server.arg("ecovid").toInt() != 0) ? 1 : 0;
      }
      if (_server.hasArg("gmask")) {
        uint32_t gm = strtoul(_server.arg("gmask").c_str(), nullptr, 10) & ((1UL << TOTAL_GAMES) - 1);
        if (gm != 0) gSettings.data.game_enabled_mask = gm;
      }
      if (_server.hasArg("gorder")) {
        String ordStr = _server.arg("gorder");
        uint8_t tmpOrd[TOTAL_GAMES];
        int idx = 0, pos = 0;
        uint32_t seen = 0;
        while (idx < TOTAL_GAMES && pos <= (int)ordStr.length()) {
          int comma = ordStr.indexOf(',', pos);
          String tok = (comma < 0) ? ordStr.substring(pos) : ordStr.substring(pos, comma);
          int val = tok.toInt();
          if (tok.length() > 0 && val >= 0 && val < TOTAL_GAMES && !(seen & (1UL << val))) {
            tmpOrd[idx++] = (uint8_t)val;
            seen |= (1UL << val);
          }
          if (comma < 0) break;
          pos = comma + 1;
        }
        if (idx == TOTAL_GAMES) {
          memcpy(gSettings.data.game_order, tmpOrd, TOTAL_GAMES);
        }
      }
      gSettings.data.idle_lock_qr = 0;
      gSettings.save();
      if (_idle) _idle->scanMediaFiles();
      _server.send(200, "text/plain", "Settings Saved");
    });

    _server.on("/api/resetscores", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      HighScoreStore::resetAllScores();
      _server.send(200, "text/plain", "Scores Reset");
    });

    // 17. Manual OTA Firmware Upload (.bin)
    _server.on("/api/ota", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      bool ok = !Update.hasError();
      _server.send(ok ? 200 : 500, "application/json",
                   ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Update failed\"}");
      if (ok) {
        delay(500);
        ESP.restart();
      }
    }, [this]() {
      gPower.notifyWebActivity();
      HTTPUpload& upload = _server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        if (_display) {
          OtaUpdater::drawScreenProgress(*_display, "Web OTA Upload", "Flashing...", 0, 0, 0, Color565::Cyan);
        }
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          if (_display) {
            OtaUpdater::drawScreenProgress(*_display, "OTA Complete!", "Rebooting...", 100,
                                           upload.totalSize / 1024, upload.totalSize / 1024, Color565::Lime);
          }
        } else {
          Update.printError(Serial);
        }
      }
    });

    // 18. Online OTA Check (GitHub Releases)
    _server.on("/api/ota_check", HTTP_GET, [this]() {
      gPower.notifyWebActivity();
      OtaReleaseInfo rel = OtaUpdater::checkGitHubRelease();
      String json = "{";
      json += "\"ok\":" + String(rel.ok ? "true" : "false") + ",";
      json += "\"current\":\"" + String(APP_VERSION) + "\",";
      json += "\"latest\":\"" + rel.latestVersion + "\",";
      json += "\"hasUpdate\":" + String(rel.hasUpdate ? "true" : "false") + ",";
      json += "\"binUrl\":\"" + rel.binUrl + "\",";
      json += "\"error\":\"" + rel.error + "\"";
      json += "}";
      _server.send(200, "application/json", json);
    });

    // 19. Online OTA Update from URL or GitHub Release
    _server.on("/api/ota_online", HTTP_POST, [this]() {
      gPower.notifyWebActivity();
      String url = _server.hasArg("url") ? _server.arg("url") : "";
      url.trim();
      if (url.length() == 0) {
        OtaReleaseInfo rel = OtaUpdater::checkGitHubRelease();
        if (!rel.ok || rel.binUrl.length() == 0) {
          _server.send(400, "application/json",
                       "{\"ok\":false,\"error\":\"" + (rel.error.length() ? rel.error : String("No release .bin found")) + "\"}");
          return;
        }
        url = rel.binUrl;
      }
      String err;
      bool ok = OtaUpdater::performUrlOta(url, _display, err);
      _server.send(ok ? 200 : 500, "application/json",
                   ok ? "{\"ok\":true}" : ("{\"ok\":false,\"error\":\"" + err + "\"}"));
      if (ok) {
        delay(500);
        ESP.restart();
      }
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
