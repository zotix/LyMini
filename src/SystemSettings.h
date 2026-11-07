#pragma once
#include <Arduino.h>
#include <Preferences.h>

#define MAX_SAVED_WIFI 3

// Idle screen bitmask flags
#define IDLE_MASK_CLOCK   0x01
#define IDLE_MASK_WEATHER 0x02
#define IDLE_MASK_PHOTO   0x04
#define IDLE_MASK_VIDEO   0x08
#define IDLE_MASK_ALL     0x0F

struct WifiCredential {
  char ssid[33];
  char pass[65];
};

struct SystemSettingsData {
  WifiCredential wifi_list[MAX_SAVED_WIFI];
  uint8_t wifi_count = 0;

  // Primary/legacy shortcut
  char wifi_ssid[33] = "";
  char wifi_pass[65] = "";

  char city_name[33] = "Bangkok";
  float latitude     = 13.7563f;
  float longitude    = 100.5018f;
  int timezone_offset= 7;          // UTC+7 (Thailand)
  
  uint8_t brightness = 85;         // 10-100%
  uint16_t idle_timeout_sec = 60;  // Default 1 minute
  uint16_t idle_cycle_sec   = 12;  // Seconds per idle screen
  uint8_t idle_enabled_mask = IDLE_MASK_ALL; // Bitmask: Clock|Weather|Photo|Video
  uint8_t idle_random_mode  = 1;   // 1 = Random screen & media, 0 = Sequential
  uint8_t led_enabled       = 1;

  // Pipe-separated list of disabled media filenames, e.g. "|01_cyber_city.rgb|"
  char disabled_media[256]  = "";
};

class SystemSettings {
public:
  SystemSettingsData data;

  void begin() {
    memset(&data, 0, sizeof(data));
    strncpy(data.city_name, "Bangkok", sizeof(data.city_name) - 1);
    data.latitude = 13.7563f;
    data.longitude = 100.5018f;
    data.timezone_offset = 7;
    data.brightness = 85;
    data.idle_timeout_sec = 60;
    data.idle_cycle_sec = 12;
    data.idle_enabled_mask = IDLE_MASK_ALL;
    data.idle_random_mode = 1;
    data.led_enabled = 1;

    Preferences prefs;
    if (prefs.begin("miniapp_cfg", true)) {
      data.wifi_count = prefs.getUChar("w_cnt", 0);
      if (data.wifi_count > MAX_SAVED_WIFI) data.wifi_count = 0;

      for (int i = 0; i < MAX_SAVED_WIFI; ++i) {
        char ks[12], kp[12];
        snprintf(ks, sizeof(ks), "w_ssid_%d", i);
        snprintf(kp, sizeof(kp), "w_pass_%d", i);
        String s = prefs.getString(ks, "");
        String p = prefs.getString(kp, "");
        strncpy(data.wifi_list[i].ssid, s.c_str(), 32);
        strncpy(data.wifi_list[i].pass, p.c_str(), 64);
      }

      // Backward compatibility with single ssid/pass
      String legacySsid = prefs.getString("ssid", "");
      String legacyPass = prefs.getString("pass", "");
      if (data.wifi_count == 0 && legacySsid.length() > 0) {
        strncpy(data.wifi_list[0].ssid, legacySsid.c_str(), 32);
        strncpy(data.wifi_list[0].pass, legacyPass.c_str(), 64);
        data.wifi_count = 1;
      }

      if (data.wifi_count > 0) {
        strncpy(data.wifi_ssid, data.wifi_list[0].ssid, 32);
        strncpy(data.wifi_pass, data.wifi_list[0].pass, 64);
      }

      String c = prefs.getString("city", "Bangkok");
      strncpy(data.city_name, c.c_str(), sizeof(data.city_name) - 1);
      
      data.latitude         = prefs.getFloat("lat", 13.7563f);
      data.longitude        = prefs.getFloat("lon", 100.5018f);
      data.timezone_offset  = prefs.getInt("tz", 7);
      data.brightness       = prefs.getUChar("bright", 85);
      data.idle_timeout_sec = prefs.getUShort("idletime", 60);
      data.idle_cycle_sec   = prefs.getUShort("idlecyc", 12);
      data.idle_enabled_mask= prefs.getUChar("idlemask", IDLE_MASK_ALL);
      if (data.idle_enabled_mask == 0) data.idle_enabled_mask = IDLE_MASK_ALL;
      data.idle_random_mode = prefs.getUChar("idlernd", 1);
      data.led_enabled      = prefs.getUChar("led", 1);

      String dis = prefs.getString("dis_med", "");
      strncpy(data.disabled_media, dis.c_str(), sizeof(data.disabled_media) - 1);
      prefs.end();
    }
  }

  void save() {
    Preferences prefs;
    if (prefs.begin("miniapp_cfg", false)) {
      prefs.putUChar("w_cnt", data.wifi_count);
      for (int i = 0; i < MAX_SAVED_WIFI; ++i) {
        char ks[12], kp[12];
        snprintf(ks, sizeof(ks), "w_ssid_%d", i);
        snprintf(kp, sizeof(kp), "w_pass_%d", i);
        prefs.putString(ks, data.wifi_list[i].ssid);
        prefs.putString(kp, data.wifi_list[i].pass);
      }
      if (data.wifi_count > 0) {
        strncpy(data.wifi_ssid, data.wifi_list[0].ssid, 32);
        strncpy(data.wifi_pass, data.wifi_list[0].pass, 64);
        prefs.putString("ssid", data.wifi_ssid);
        prefs.putString("pass", data.wifi_pass);
      }

      prefs.putString("city", data.city_name);
      prefs.putFloat("lat", data.latitude);
      prefs.putFloat("lon", data.longitude);
      prefs.putInt("tz", data.timezone_offset);
      prefs.putUChar("bright", data.brightness);
      prefs.putUShort("idletime", data.idle_timeout_sec);
      prefs.putUShort("idlecyc", data.idle_cycle_sec);
      prefs.putUChar("idlemask", data.idle_enabled_mask);
      prefs.putUChar("idlernd", data.idle_random_mode);
      prefs.putUChar("led", data.led_enabled);
      prefs.putString("dis_med", data.disabled_media);
      prefs.end();
    }
  }

  void addOrUpdateWifi(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return;
    // Check if already exists
    for (int i = 0; i < data.wifi_count; ++i) {
      if (strcmp(data.wifi_list[i].ssid, ssid) == 0) {
        if (pass && strlen(pass) > 0) {
          strncpy(data.wifi_list[i].pass, pass, 64);
        }
        // Move to slot 0 (most recent)
        WifiCredential tmp = data.wifi_list[i];
        for (int j = i; j > 0; --j) data.wifi_list[j] = data.wifi_list[j - 1];
        data.wifi_list[0] = tmp;
        save();
        return;
      }
    }
    // Insert at front
    int newCount = (data.wifi_count < MAX_SAVED_WIFI) ? (data.wifi_count + 1) : MAX_SAVED_WIFI;
    for (int j = newCount - 1; j > 0; --j) {
      data.wifi_list[j] = data.wifi_list[j - 1];
    }
    memset(&data.wifi_list[0], 0, sizeof(WifiCredential));
    strncpy(data.wifi_list[0].ssid, ssid, 32);
    if (pass) strncpy(data.wifi_list[0].pass, pass, 64);
    data.wifi_count = newCount;
    save();
  }

  void removeWifi(int index) {
    if (index < 0 || index >= data.wifi_count) return;
    for (int i = index; i < data.wifi_count - 1; ++i) {
      data.wifi_list[i] = data.wifi_list[i + 1];
    }
    memset(&data.wifi_list[data.wifi_count - 1], 0, sizeof(WifiCredential));
    data.wifi_count--;
    save();
  }

  bool isMediaEnabled(const char* filename) const {
    if (!filename || strlen(data.disabled_media) == 0) return true;
    // Extract basename
    const char* base = strrchr(filename, '/');
    base = base ? (base + 1) : filename;
    char token[48];
    snprintf(token, sizeof(token), "|%s|", base);
    return (strstr(data.disabled_media, token) == nullptr);
  }

  void setMediaEnabled(const char* filename, bool enabled) {
    if (!filename) return;
    const char* base = strrchr(filename, '/');
    base = base ? (base + 1) : filename;
    char token[48];
    snprintf(token, sizeof(token), "|%s|", base);

    String cur(data.disabled_media);
    if (enabled) {
      cur.replace(token, "");
    } else {
      if (cur.indexOf(token) < 0) {
        cur += token;
      }
    }
    strncpy(data.disabled_media, cur.c_str(), sizeof(data.disabled_media) - 1);
    save();
  }
};

extern SystemSettings gSettings;
