#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include "Config.h"
#include "DisplayCanvas.h"
#include "InputManager.h"
#include "PowerManager.h"

struct OtaReleaseInfo {
  bool   ok = false;
  bool   hasUpdate = false;
  String latestVersion = "";
  String binUrl = "";
  String error = "";
};

class OtaUpdater {
private:
  bool _modalActive = false;
  uint8_t _modalStage = 0; // 0=Confirm, 1=Result/Error wait key
  String _statusLine1 = "";
  String _statusLine2 = "";
  String _statusLine3 = "";
  bool _isError = false;

  static int parseVerPart(const String& s, int& idx) {
    int n = 0;
    while (idx < (int)s.length() && !isdigit((unsigned char)s[idx])) idx++;
    while (idx < (int)s.length() && isdigit((unsigned char)s[idx])) {
      n = n * 10 + (s[idx] - '0');
      idx++;
    }
    return n;
  }

public:
  static String normalizeVersion(String v) {
    v.trim();
    if (v.startsWith("v") || v.startsWith("V")) v = v.substring(1);
    return v;
  }

  static bool isNewerVersion(const String& latestRaw, const String& currentRaw) {
    String a = normalizeVersion(latestRaw);
    String b = normalizeVersion(currentRaw);
    int ia = 0, ib = 0;
    for (int k = 0; k < 3; ++k) {
      int va = parseVerPart(a, ia);
      int vb = parseVerPart(b, ib);
      if (va > vb) return true;
      if (va < vb) return false;
    }
    return false;
  }

  static String extractJsonString(const String& json, const char* key, int startFrom = 0) {
    String pat = String("\"") + key + "\":\"";
    int p = json.indexOf(pat, startFrom);
    if (p < 0) return "";
    p += pat.length();
    int end = json.indexOf('"', p);
    if (end < 0) return "";
    return json.substring(p, end);
  }

  static OtaReleaseInfo checkGitHubRelease() {
    OtaReleaseInfo info;
    if (WiFi.status() != WL_CONNECTED) {
      info.error = "Wi-Fi not connected";
      return info;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setTimeout(10000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    String apiUrl = String("https://api.github.com/repos/") + OTA_GITHUB_REPO + "/releases/latest";

    if (!http.begin(client, apiUrl)) {
      info.error = "HTTP begin failed";
      return info;
    }
    http.addHeader("User-Agent", "LyMini-ESP32-OTA");
    http.addHeader("Accept", "application/vnd.github+json");

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
      info.error = "GitHub HTTP " + String(code);
      http.end();
      return info;
    }

    String payload = http.getString();
    http.end();

    String tag = extractJsonString(payload, "tag_name");
    if (tag.length() == 0) {
      info.error = "No release tag found";
      return info;
    }

    info.latestVersion = normalizeVersion(tag);

    // Find first .bin browser_download_url
    int searchPos = 0;
    while (true) {
      int idx = payload.indexOf("\"browser_download_url\":\"", searchPos);
      if (idx < 0) break;
      String u = extractJsonString(payload, "browser_download_url", idx);
      if (u.endsWith(".bin")) {
        info.binUrl = u;
        break;
      }
      searchPos = idx + 24;
    }

    info.ok = true;
    info.hasUpdate = isNewerVersion(info.latestVersion, APP_VERSION) && (info.binUrl.length() > 0);
    if (isNewerVersion(info.latestVersion, APP_VERSION) && info.binUrl.length() == 0) {
      info.error = "No .bin asset in release";
    }
    return info;
  }

  static void drawScreenProgress(DisplayCanvas& gfx, const char* title, const char* sub,
                                 int pct, uint32_t doneKB, uint32_t totalKB, uint16_t barCol) {
    gfx.clear(Color565::Black);
    gfx.fillRect(0, 0, SCREEN_WIDTH, 18, Color565::Navy);
    gfx.drawCenteredText(5, "OTA SYSTEM UPDATE", Color565::Cyan);

    gfx.drawCenteredText(34, title, Color565::White);
    gfx.drawCenteredText(50, sub, Color565::GrayLight);

    // Progress bar
    gfx.drawRect(10, 72, 108, 14, Color565::GrayMid);
    if (pct >= 0) {
      int fillW = (constrain(pct, 0, 100) * 104) / 100;
      if (fillW > 0) gfx.fillRect(12, 74, fillW, 10, barCol);
      char pbuf[28];
      if (totalKB > 0) {
        snprintf(pbuf, sizeof(pbuf), "%d%% (%lu/%lu KB)", pct, (unsigned long)doneKB, (unsigned long)totalKB);
      } else {
        snprintf(pbuf, sizeof(pbuf), "%d%% (%lu KB)", pct, (unsigned long)doneKB);
      }
      gfx.drawCenteredText(94, pbuf, Color565::Gold);
    }
    gfx.present();
  }

  static bool performUrlOta(const String& url, DisplayCanvas* gfx, String& errOut) {
    if (WiFi.status() != WL_CONNECTED) {
      errOut = "Wi-Fi not connected";
      return false;
    }
    if (url.length() == 0) {
      errOut = "Empty firmware URL";
      return false;
    }

    if (gfx) {
      drawScreenProgress(*gfx, "Connecting...", "Downloading .bin", 0, 0, 0, Color565::Cyan);
    }

    WiFiClientSecure secClient;
    secClient.setInsecure();
    WiFiClient plainClient;
    HTTPClient http;
    http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    bool begun = url.startsWith("https://")
      ? http.begin(secClient, url)
      : http.begin(plainClient, url);

    if (!begun) {
      errOut = "HTTP begin failed";
      return false;
    }
    http.addHeader("User-Agent", "LyMini-ESP32-OTA");

    int code = http.GET();
    if (code != HTTP_CODE_OK) {
      errOut = "Download HTTP " + String(code);
      http.end();
      return false;
    }

    int contentLen = http.getSize();
    if (!Update.begin(contentLen > 0 ? (size_t)contentLen : UPDATE_SIZE_UNKNOWN)) {
      errOut = "Update.begin failed";
      http.end();
      return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    uint8_t buf[1024];
    size_t written = 0;
    uint32_t lastDrawMs = 0;
    uint32_t lastDataMs = millis();

    while (http.connected() && (contentLen <= 0 || written < (size_t)contentLen)) {
      size_t avail = stream->available();
      if (avail) {
        int toRead = (avail > sizeof(buf)) ? sizeof(buf) : (int)avail;
        int rd = stream->readBytes(buf, toRead);
        if (rd > 0) {
          if (Update.write(buf, rd) != (size_t)rd) {
            errOut = "Flash write error";
            Update.abort();
            http.end();
            return false;
          }
          written += rd;
          lastDataMs = millis();

          if (gfx && (millis() - lastDrawMs >= 200 || (contentLen > 0 && written >= (size_t)contentLen))) {
            lastDrawMs = millis();
            int pct = (contentLen > 0) ? (int)((written * 100ULL) / (size_t)contentLen) : 50;
            drawScreenProgress(*gfx, "Updating Firmware", "Do NOT power off!",
                               pct, written / 1024, (contentLen > 0) ? (contentLen / 1024) : 0, Color565::Green);
          }
        }
      } else {
        if (millis() - lastDataMs > 15000) {
          errOut = "Download timeout";
          Update.abort();
          http.end();
          return false;
        }
        delay(2);
      }
    }

    http.end();

    if (!Update.end(true)) {
      errOut = "Update.end err " + String(Update.getError());
      return false;
    }

    if (gfx) {
      drawScreenProgress(*gfx, "Update Complete!", "Rebooting...", 100, written / 1024, written / 1024, Color565::Lime);
    }
    return true;
  }

  void openConfirmModal() {
    _modalActive = true;
    _modalStage = 0;
    _isError = false;
  }

  bool isModalActive() const { return _modalActive; }

  // Returns true while modal owns the screen
  bool updateAndRender(const InputState& input, DisplayCanvas& gfx) {
    if (!_modalActive) return false;

    if (_modalStage == 0) {
      // Confirm screen: A = Confirm & Check, B or BOOT = Cancel
      if (input.leftPressed) {
        // Ensure Wi-Fi is ON and try to connect if needed
        if (WiFi.status() != WL_CONNECTED) {
          gPower.requestWifiOn();
          uint32_t waitStart = millis();
          while (WiFi.status() != WL_CONNECTED && millis() - waitStart < 8000) {
            drawScreenProgress(gfx, "Connecting Wi-Fi...", "Please wait", -1, 0, 0, Color565::Cyan);
            delay(100);
          }
        }

        if (WiFi.status() != WL_CONNECTED) {
          _modalStage = 1;
          _isError = true;
          _statusLine1 = "No Wi-Fi Internet";
          _statusLine2 = "Connect STA Wi-Fi";
          _statusLine3 = "in Web Portal first";
          return true;
        }

        drawScreenProgress(gfx, "Checking Online...", "GitHub Releases", -1, 0, 0, Color565::Cyan);
        OtaReleaseInfo rel = checkGitHubRelease();
        if (!rel.ok) {
          _modalStage = 1;
          _isError = true;
          _statusLine1 = "Check Failed!";
          _statusLine2 = rel.error;
          _statusLine3 = "Press any key";
          return true;
        }

        if (!rel.hasUpdate) {
          _modalStage = 1;
          _isError = false;
          _statusLine1 = "Already Up To Date";
          _statusLine2 = "Cur: v" + String(APP_VERSION);
          _statusLine3 = "New: v" + rel.latestVersion;
          return true;
        }

        String err;
        if (performUrlOta(rel.binUrl, &gfx, err)) {
          delay(1200);
          ESP.restart();
        } else {
          _modalStage = 1;
          _isError = true;
          _statusLine1 = "OTA Failed!";
          _statusLine2 = err;
          _statusLine3 = "Press any key";
        }
        return true;
      } else if (input.rightPressed || input.okShortReleased) {
        _modalActive = false;
        return false;
      }

      // Render Confirmation Screen
      gfx.clear(Color565::Black);
      gfx.fillRect(0, 0, SCREEN_WIDTH, 18, Color565::Navy);
      gfx.drawCenteredText(5, "OTA UPDATE?", Color565::Gold);

      char vbuf[28];
      snprintf(vbuf, sizeof(vbuf), "Current: v%s", APP_VERSION);
      gfx.drawCenteredText(28, vbuf, Color565::White);

      if (WiFi.status() == WL_CONNECTED) {
        String wline = "WiFi: " + WiFi.SSID();
        if (wline.length() > 20) wline = wline.substring(0, 20);
        gfx.drawCenteredText(44, wline.c_str(), Color565::Lime);
      } else {
        gfx.drawCenteredText(44, "WiFi: Auto-Connect", Color565::Orange);
      }

      gfx.drawCenteredText(64, "Check GitHub &", Color565::GrayLight);
      gfx.drawCenteredText(76, "Install Latest FW", Color565::GrayLight);

      gfx.fillRect(4, 96, 120, 13, Color565::Emerald);
      gfx.drawCenteredText(99, "A: CHECK & UPDATE", Color565::White);

      gfx.fillRect(4, 112, 120, 13, Color565::Crimson);
      gfx.drawCenteredText(115, "B/BOOT: CANCEL", Color565::White);
      gfx.present();
      return true;
    }

    // Stage 1: Result / Error screen — press any key to close
    if (input.leftPressed || input.rightPressed || input.okShortReleased) {
      _modalActive = false;
      return false;
    }

    gfx.clear(Color565::Black);
    gfx.fillRect(0, 0, SCREEN_WIDTH, 18, _isError ? Color565::Crimson : Color565::Navy);
    gfx.drawCenteredText(5, _isError ? "OTA ERROR" : "OTA STATUS", Color565::White);
    gfx.drawCenteredText(38, _statusLine1.c_str(), _isError ? Color565::Orange : Color565::Lime);
    gfx.drawCenteredText(56, _statusLine2.c_str(), Color565::White);
    gfx.drawCenteredText(72, _statusLine3.c_str(), Color565::GrayLight);
    gfx.drawCenteredText(110, "Press any button", Color565::Gold);
    gfx.present();
    return true;
  }
};
