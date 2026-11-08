#pragma once
#include <Arduino.h>
#include <Preferences.h>

#define MAX_SAVED_WIFI     3
#define MAX_WEATHER_CITIES 5

// Idle screen bitmask flags (8 screens in v0.2.2)
#define IDLE_MASK_CLOCK       0x01 // 0: Cyber Digital Clock
#define IDLE_MASK_WEATHER     0x02 // 1: Weather & Temp
#define IDLE_MASK_PHOTO       0x04 // 2: Photo Frame
#define IDLE_MASK_VIDEO       0x08 // 3: Video / Animation
#define IDLE_MASK_ANALOG      0x10 // 4: Analog Clock
#define IDLE_MASK_FLIP        0x20 // 5: Flip Clock
#define IDLE_MASK_ABOUT_QR    0x40 // 6: About Me & PromptPay QR (Mandatory, always enabled)
#define IDLE_MASK_CLK_WEATHER 0x80 // 7: Clock + Weather Combo Screen
#define IDLE_MASK_ALL         0xFF

struct WifiCredential {
  char ssid[33];
  char pass[65];
};

struct WeatherLocation {
  char  name[28];
  float lat;
  float lon;
};

struct SystemSettingsData {
  WifiCredential wifi_list[MAX_SAVED_WIFI];
  uint8_t wifi_count = 0;

  char wifi_ssid[33] = "";
  char wifi_pass[65] = "";

  // Multi-location weather list (up to 5 locations)
  WeatherLocation weather_locs[MAX_WEATHER_CITIES];
  uint8_t weather_count = 1;

  char city_name[33] = "Bangkok";
  float latitude     = 13.7563f;
  float longitude    = 100.5018f;
  int timezone_offset= 7;          // UTC+7 (Thailand)
  
  uint8_t brightness = 85;         // 5-100% Max Brightness ceiling
  uint8_t eco_brightness = 25;     // 5-100% Eco Power-Saving Brightness (25% at >= 5 min)
  uint8_t screen_rotation = 3;     // 0=0°, 1=90°, 2=180°, 3=270° (Default in v0.2.2)
  uint8_t btn_rotate_with_screen = 1; // 1=Rotate Left/Right buttons with screen orientation
  uint8_t color_order_bgr = 1;     // 1=BGR (ST7735 GREENTAB3 standard with MSB swap), 0=RGB

  uint16_t idle_timeout_sec = 30;  // Default 30 seconds
  uint16_t eco_timeout_sec  = 300; // Default 300 seconds (5 mins)
  uint16_t idle_cycle_sec   = 12;  // Seconds per idle screen
  uint8_t idle_enabled_mask = IDLE_MASK_ALL;
  uint8_t idle_random_mode  = 1;   // 1 = Random screen & media, 0 = Sequential
  uint8_t idle_lock_qr      = 0;   // 1 = Lock Idle on PromptPay QR code
  uint8_t led_enabled       = 1;

  // Night mode screen-off schedule
  uint8_t night_mode_enabled = 0;
  uint8_t night_start_hour   = 23; // 23:00
  uint8_t night_end_hour     = 6;  // 06:00

  // Pipe-separated list of disabled media filenames
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

    data.weather_count = 1;
    strncpy(data.weather_locs[0].name, "Bangkok", sizeof(data.weather_locs[0].name) - 1);
    data.weather_locs[0].lat = 13.7563f;
    data.weather_locs[0].lon = 100.5018f;

    data.brightness = 85;
    data.eco_brightness = 35;
    data.screen_rotation = 3; // Default 270 degrees
    data.btn_rotate_with_screen = 1;
    data.color_order_bgr = 1;
    data.idle_timeout_sec = 30;
    data.idle_cycle_sec = 12;
    data.idle_enabled_mask = IDLE_MASK_ALL;
    data.idle_random_mode = 1;
    data.idle_lock_qr = 0;
    data.led_enabled = 1;
    data.night_mode_enabled = 0;
    data.night_start_hour = 23;
    data.night_end_hour = 6;

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
      data.latitude  = prefs.getFloat("lat", 13.7563f);
      data.longitude = prefs.getFloat("lon", 100.5018f);

      // Load multi-location weather list
      data.weather_count = prefs.getUChar("wt_cnt", 0);
      if (data.weather_count > MAX_WEATHER_CITIES) data.weather_count = 0;
      for (int i = 0; i < data.weather_count; ++i) {
        char kn[12], klat[12], klon[12];
        snprintf(kn, sizeof(kn), "wt_n_%d", i);
        snprintf(klat, sizeof(klat), "wt_la_%d", i);
        snprintf(klon, sizeof(klon), "wt_lo_%d", i);
        String wn = prefs.getString(kn, "");
        strncpy(data.weather_locs[i].name, wn.c_str(), sizeof(data.weather_locs[i].name) - 1);
        data.weather_locs[i].lat = prefs.getFloat(klat, 13.7563f);
        data.weather_locs[i].lon = prefs.getFloat(klon, 100.5018f);
      }
      if (data.weather_count == 0) {
        data.weather_count = 1;
        strncpy(data.weather_locs[0].name, data.city_name, sizeof(data.weather_locs[0].name) - 1);
        data.weather_locs[0].lat = data.latitude;
        data.weather_locs[0].lon = data.longitude;
      }

      data.timezone_offset        = prefs.getInt("tz", 7);
      data.brightness             = constrain(prefs.getUChar("bright", 85), 5, 100);
      data.eco_brightness         = constrain(prefs.getUChar("ecobri", 25), 5, 100);
      data.screen_rotation        = prefs.getUChar("rot2", 3); // Default 3 (270 deg in v0.2.2)
      data.btn_rotate_with_screen = prefs.getUChar("btnrot", 1);
      data.color_order_bgr        = prefs.getUChar("bgr", 1);
      data.idle_timeout_sec       = prefs.getUShort("idletime2", 30);
      data.eco_timeout_sec        = prefs.getUShort("ecotime", 300);
      data.idle_cycle_sec         = prefs.getUShort("idlecyc", 12);
      data.idle_enabled_mask      = prefs.getUChar("idlemask3", IDLE_MASK_ALL);
      // PromptPay QR is mandatory and cannot be unchecked
      data.idle_enabled_mask |= IDLE_MASK_ABOUT_QR;

      data.idle_random_mode  = prefs.getUChar("idlernd", 1);
      data.idle_lock_qr      = 0;
      data.led_enabled       = prefs.getUChar("led", 1);
      data.night_mode_enabled= prefs.getUChar("night_en", 0);
      data.night_start_hour  = prefs.getUChar("night_st", 23);
      data.night_end_hour    = prefs.getUChar("night_ed", 6);

      String dis = prefs.getString("dis_med", "");
      strncpy(data.disabled_media, dis.c_str(), sizeof(data.disabled_media) - 1);
      prefs.end();
    }
  }

  void save() {
    // Ensure PromptPay QR is always enabled
    data.idle_enabled_mask |= IDLE_MASK_ABOUT_QR;
    data.brightness     = constrain(data.brightness, 5, 100);
    data.eco_brightness = constrain(data.eco_brightness, 5, 100);

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

      prefs.putUChar("wt_cnt", data.weather_count);
      for (int i = 0; i < MAX_WEATHER_CITIES; ++i) {
        char kn[12], klat[12], klon[12];
        snprintf(kn, sizeof(kn), "wt_n_%d", i);
        snprintf(klat, sizeof(klat), "wt_la_%d", i);
        snprintf(klon, sizeof(klon), "wt_lo_%d", i);
        prefs.putString(kn, data.weather_locs[i].name);
        prefs.putFloat(klat, data.weather_locs[i].lat);
        prefs.putFloat(klon, data.weather_locs[i].lon);
      }

      if (data.weather_count > 0) {
        strncpy(data.city_name, data.weather_locs[0].name, sizeof(data.city_name) - 1);
        data.latitude  = data.weather_locs[0].lat;
        data.longitude = data.weather_locs[0].lon;
      }

      prefs.putString("city", data.city_name);
      prefs.putFloat("lat", data.latitude);
      prefs.putFloat("lon", data.longitude);
      prefs.putInt("tz", data.timezone_offset);
      prefs.putUChar("bright", data.brightness);
      prefs.putUChar("ecobri", data.eco_brightness);
      prefs.putUChar("rot2", data.screen_rotation);
      prefs.putUChar("btnrot", data.btn_rotate_with_screen);
      prefs.putUChar("bgr", data.color_order_bgr);
      prefs.putUShort("idletime2", data.idle_timeout_sec);
      prefs.putUShort("ecotime", data.eco_timeout_sec);
      prefs.putUShort("idlecyc", data.idle_cycle_sec);
      prefs.putUChar("idlemask3", data.idle_enabled_mask);
      prefs.putUChar("idlernd", data.idle_random_mode);
      prefs.putUChar("lockqr", data.idle_lock_qr);
      prefs.putUChar("led", data.led_enabled);
      prefs.putUChar("night_en", data.night_mode_enabled);
      prefs.putUChar("night_st", data.night_start_hour);
      prefs.putUChar("night_ed", data.night_end_hour);
      prefs.putString("dis_med", data.disabled_media);
      prefs.end();
    }
  }

  void addOrUpdateWifi(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return;
    for (int i = 0; i < data.wifi_count; ++i) {
      if (strcmp(data.wifi_list[i].ssid, ssid) == 0) {
        if (pass && strlen(pass) > 0) {
          strncpy(data.wifi_list[i].pass, pass, 64);
        }
        WifiCredential tmp = data.wifi_list[i];
        for (int j = i; j > 0; --j) data.wifi_list[j] = data.wifi_list[j - 1];
        data.wifi_list[0] = tmp;
        save();
        return;
      }
    }
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

  int addOrUpdateWeatherCity(const char* name, float lat, float lon) {
    if (!name || strlen(name) == 0) return -1;
    for (int i = 0; i < data.weather_count; ++i) {
      if (strcasecmp(data.weather_locs[i].name, name) == 0) {
        data.weather_locs[i].lat = lat;
        data.weather_locs[i].lon = lon;
        save();
        return i;
      }
    }
    if (data.weather_count < MAX_WEATHER_CITIES) {
      int idx = data.weather_count++;
      memset(&data.weather_locs[idx], 0, sizeof(WeatherLocation));
      strncpy(data.weather_locs[idx].name, name, sizeof(data.weather_locs[idx].name) - 1);
      data.weather_locs[idx].lat = lat;
      data.weather_locs[idx].lon = lon;
      save();
      return idx;
    }
    // Replace last slot if full
    int idx = MAX_WEATHER_CITIES - 1;
    strncpy(data.weather_locs[idx].name, name, sizeof(data.weather_locs[idx].name) - 1);
    data.weather_locs[idx].lat = lat;
    data.weather_locs[idx].lon = lon;
    save();
    return idx;
  }

  void removeWeatherCity(int index) {
    if (index < 0 || index >= data.weather_count || data.weather_count <= 1) return;
    for (int i = index; i < data.weather_count - 1; ++i) {
      data.weather_locs[i] = data.weather_locs[i + 1];
    }
    data.weather_count--;
    save();
  }

  bool isMediaEnabled(const char* filename) const {
    if (!filename || strlen(data.disabled_media) == 0) return true;
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
