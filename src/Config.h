#pragma once
#include <Arduino.h>

#define APP_VERSION "0.3.3"
#define OTA_GITHUB_REPO "zotix/LyMini"

// ============================================================================
// Spotpear ESP32-C3 1.44" Hardware Pin Definitions (v0.2.1)
// ============================================================================

// Display pins (ST7735 128x128 SPI)
#define PIN_TFT_MOSI 4   // SPI MOSI / SDA
#define PIN_TFT_SCLK 3   // SPI SCK / SCL
#define PIN_TFT_CS   2   // Chip Select
#define PIN_TFT_DC   0   // Data / Command
#define PIN_TFT_RST  5   // Display Reset
#define PIN_TFT_BL   7   // Backlight PWM

// Buttons (Active LOW with internal pull-up)
// Updated in v0.2.1: Button A/Left = IO10, Button B/Right = IO8
#define PIN_BTN_LEFT  10  // Button A / Left (IO10)
#define PIN_BTN_RIGHT 8   // Button B / Right (IO8)
#define PIN_BTN_OK    9   // BOOT (B) button (IO9)

// Hold durations
#define BOOT_HOLD_MENU_EXIT_MS 4000UL // Hold BOOT 4 seconds in-game to return to menu
#define BOOT_HOLD_LAUNCHER_MS  1500UL // Hold BOOT 1.5s in menu to enter Idle TV
#define BOOT_HOLD_OTA_MS       10000UL // Hold BOOT 10s anywhere to open OTA Update confirmation

// Status / Game FX LED
#define PIN_STATUS_LED 11 // LED L191 (Active HIGH)

// Screen Geometry
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Backlight PWM Settings
#define BL_PWM_FREQ   20000
#define BL_PWM_RES    8
#define BL_PWM_CHAN   0

// ============================================================================
// Vibrant 16-bit RGB565 Color Palette
// ============================================================================
namespace Color565 {
  constexpr uint16_t Black       = 0x0000;
  constexpr uint16_t White       = 0xFFFF;
  constexpr uint16_t GrayDark    = 0x2104;
  constexpr uint16_t GrayMid     = 0x7BEF;
  constexpr uint16_t GrayLight   = 0xBDD7;
  
  constexpr uint16_t Red         = 0xF800;
  constexpr uint16_t Crimson     = 0xD8A3;
  constexpr uint16_t Coral       = 0xFBE4;
  constexpr uint16_t Orange      = 0xFD20;
  constexpr uint16_t Gold        = 0xFEA0;
  constexpr uint16_t Yellow      = 0xFFE0;
  
  constexpr uint16_t Green       = 0x07E0;
  constexpr uint16_t Lime        = 0x87E0;
  constexpr uint16_t Mint        = 0x47F1;
  constexpr uint16_t Emerald     = 0x05EB;
  
  constexpr uint16_t Cyan        = 0x07FF;
  constexpr uint16_t SkyBlue     = 0x555F;
  constexpr uint16_t Blue        = 0x001F;
  constexpr uint16_t DeepBlue    = 0x0013;
  constexpr uint16_t Navy        = 0x10B1;
  constexpr uint16_t NeonCyan    = 0x07FB;

  constexpr uint16_t Magenta     = 0xF81F;
  constexpr uint16_t Purple      = 0x901F;
  constexpr uint16_t Violet      = 0x7819;
  constexpr uint16_t PinkHot     = 0xFC18;

  inline constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }
}
