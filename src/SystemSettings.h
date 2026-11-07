#pragma once
#include <Arduino.h>
#include <Preferences.h>

struct SystemSettingsData {
  char wifi_ssid[33] = "";
  char wifi_pass[65] = "";
  char city_name[33] = "Bangkok";
  float latitude     = 13.7563f;
  float longitude    = 100.5018f;
  int timezone_offset= 7;          // UTC+7 (Thailand)
  
  uint8_t brightness = 85;         // 10-100%
  uint16_t idle_timeout_sec = 60;  // Default 1 minute
  uint8_t idle_mode  = 0;          // 0: Cycle All, 1: Clock, 2: Weather, 3: Photos, 4: Videos
  uint8_t led_enabled= 1;
};

class SystemSettings {
public:
  SystemSettingsData data;

  void begin() {
    Preferences prefs;
    if (prefs.begin("miniapp_cfg", true)) {
      String s = prefs.getString("ssid", "");
      String p = prefs.getString("pass", "");
      String c = prefs.getString("city", "Bangkok");
      
      strncpy(data.wifi_ssid, s.c_str(), sizeof(data.wifi_ssid) - 1);
      strncpy(data.wifi_pass, p.c_str(), sizeof(data.wifi_pass) - 1);
      strncpy(data.city_name, c.c_str(), sizeof(data.city_name) - 1);
      
      data.latitude = prefs.getFloat("lat", 13.7563f);
      data.longitude = prefs.getFloat("lon", 100.5018f);
      data.timezone_offset = prefs.getInt("tz", 7);
      data.brightness = prefs.getUChar("bright", 85);
      data.idle_timeout_sec = prefs.getUShort("idletime", 60);
      data.idle_mode = prefs.getUChar("idlemode", 0);
      data.led_enabled = prefs.getUChar("led", 1);
      prefs.end();
    }
  }

  void save() {
    Preferences prefs;
    if (prefs.begin("miniapp_cfg", false)) {
      prefs.putString("ssid", data.wifi_ssid);
      prefs.putString("pass", data.wifi_pass);
      prefs.putString("city", data.city_name);
      prefs.putFloat("lat", data.latitude);
      prefs.putFloat("lon", data.longitude);
      prefs.putInt("tz", data.timezone_offset);
      prefs.putUChar("bright", data.brightness);
      prefs.putUShort("idletime", data.idle_timeout_sec);
      prefs.putUChar("idlemode", data.idle_mode);
      prefs.putUChar("led", data.led_enabled);
      prefs.end();
    }
  }
};

extern SystemSettings gSettings;
