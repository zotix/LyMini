#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <math.h>
#include "Config.h"
#include "DisplayCanvas.h"

class SampleMediaSeeder {
private:
  static inline uint8_t rgb565ToRgb332(uint16_t c) {
    // Note: DisplayCanvas stores native little-endian uint16_t RGB565
    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >> 5) & 0x3F;
    uint8_t b = c & 0x1F;
    return ((r >> 2) << 5) | ((g >> 3) << 2) | (b >> 3);
  }

  static void saveCanvasAsRgb565(DisplayCanvas& gfx, const char* path) {
    File f = LittleFS.open(path, "w");
    if (!f) return;
    uint16_t* buf = gfx.getBuffer();
    // Write in big-endian RGB565 (matching Web Portal upload format) as 256-byte chunks
    uint8_t lineBuf[SCREEN_WIDTH * 2];
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
      for (int x = 0; x < SCREEN_WIDTH; ++x) {
        uint16_t c = buf[y * SCREEN_WIDTH + x];
        lineBuf[x * 2]     = (c >> 8) & 0xFF;
        lineBuf[x * 2 + 1] = c & 0xFF;
      }
      f.write(lineBuf, sizeof(lineBuf));
    }
    f.close();
  }

  static void writeCanvasAsMvidFrame332(DisplayCanvas& gfx, File& f) {
    uint16_t* buf = gfx.getBuffer();
    uint8_t lineBuf[SCREEN_WIDTH];
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
      for (int x = 0; x < SCREEN_WIDTH; ++x) {
        lineBuf[x] = rgb565ToRgb332(buf[y * SCREEN_WIDTH + x]);
      }
      f.write(lineBuf, sizeof(lineBuf));
    }
  }

  // 1. Cyber City Skyline at Night
  static void drawCyberCity(DisplayCanvas& gfx) {
    gfx.fillGradientV(0, 0, 128, 84, Color565::rgb(12, 8, 36), Color565::rgb(90, 20, 90));
    // Neon Moon
    gfx.fillCircle(96, 26, 14, Color565::rgb(255, 220, 90));
    gfx.fillCircle(96, 26, 11, Color565::rgb(255, 250, 160));
    // Stars
    const uint8_t stars[][2] = {{14,12},{28,22},{46,10},{64,18},{18,36},{52,32},{116,14}};
    for (auto& s : stars) gfx.drawPixel(s[0], s[1], Color565::White);

    // Skyline buildings
    const int16_t bx[]  = {2, 18, 34, 52, 72, 90, 108};
    const int16_t bw[]  = {14, 15, 16, 18, 16, 16, 18};
    const int16_t bh[]  = {42, 54, 38, 62, 46, 52, 36};
    for (int i = 0; i < 7; ++i) {
      int16_t topY = 84 - bh[i];
      gfx.fillRect(bx[i], topY, bw[i], bh[i], Color565::rgb(16, 18, 38));
      gfx.drawRect(bx[i], topY, bw[i], bh[i], (i % 2 == 0) ? Color565::NeonCyan : Color565::Magenta);
      // Lit windows
      for (int wy = topY + 4; wy < 80; wy += 6) {
        for (int wx = bx[i] + 3; wx < bx[i] + bw[i] - 3; wx += 4) {
          if (((wx + wy + i) % 3) != 0) {
            uint16_t wc = ((wy / 6) % 2 == 0) ? Color565::NeonCyan : Color565::Yellow;
            gfx.fillRect(wx, wy, 2, 3, wc);
          }
        }
      }
    }

    // Water reflection
    gfx.fillGradientV(0, 84, 128, 44, Color565::rgb(25, 12, 50), Color565::rgb(6, 6, 18));
    gfx.drawFastHLine(0, 84, 128, Color565::NeonCyan);
    for (int y = 88; y < 126; y += 3) {
      int16_t w = 24 - (y - 88) / 3;
      gfx.drawFastHLine(96 - w / 2, y, w, (y % 6 == 0) ? Color565::Gold : Color565::Magenta);
      gfx.drawFastHLine(20 + (y % 7) * 4, y, 12, Color565::NeonCyan);
    }
    gfx.drawCenteredText(4, "CYBER CITY 2088", Color565::NeonCyan);
  }

  // 2. Space Astronaut Frame (angle for animation or static)
  static void drawAstronautFrame(DisplayCanvas& gfx, float angle) {
    gfx.fillGradientV(0, 0, 128, 128, Color565::rgb(6, 8, 22), Color565::rgb(22, 10, 44));

    // Twinkling stars
    for (int i = 0; i < 24; ++i) {
      int16_t sx = (i * 37 + (int)(angle * 20)) % 126 + 1;
      int16_t sy = (i * 53) % 120 + 4;
      gfx.drawPixel(sx, sy, (i % 3 == 0) ? Color565::NeonCyan : Color565::White);
    }

    // Orbiting ringed planet
    int16_t px = 98, py = 28;
    gfx.drawCircle(px, py, 16, Color565::Gold);
    gfx.fillCircle(px, py, 10, Color565::Coral);
    gfx.drawLine(px - 18, py + 6, px + 18, py - 6, Color565::Yellow);

    // Floating Astronaut bobbing & rotating offset
    int16_t cx = 60 + (int16_t)(sinf(angle) * 8.0f);
    int16_t cy = 66 + (int16_t)(cosf(angle * 2.0f) * 5.0f);

    // Backpack
    gfx.fillRoundRect(cx - 16, cy - 10, 32, 26, 5, Color565::GrayLight);
    // Suit torso
    gfx.fillRoundRect(cx - 12, cy - 6, 24, 24, 5, Color565::White);
    // Chest control box
    gfx.fillRect(cx - 6, cy + 1, 12, 8, Color565::DeepBlue);
    gfx.fillRect(cx - 4, cy + 3, 3, 4, Color565::Red);
    gfx.fillRect(cx + 1, cy + 3, 3, 4, Color565::Lime);

    // Helmet
    gfx.fillCircle(cx, cy - 18, 14, Color565::White);
    int16_t visorShift = (int16_t)(sinf(angle) * 4.0f);
    gfx.fillRoundRect(cx - 9 + visorShift, cy - 24, 18, 12, 4, Color565::Gold);
    gfx.drawFastHLine(cx - 6 + visorShift, cy - 22, 8, Color565::Yellow);

    // Arms & Legs waving
    int16_t armWave = (int16_t)(cosf(angle) * 6.0f);
    gfx.fillRoundRect(cx - 21, cy - 4 + armWave, 8, 6, 2, Color565::White);
    gfx.fillRoundRect(cx + 13, cy - 4 - armWave, 8, 6, 2, Color565::White);
    gfx.fillRoundRect(cx - 10, cy + 18, 8, 10, 2, Color565::White);
    gfx.fillRoundRect(cx + 2,  cy + 18, 8, 10, 2, Color565::White);
  }

  // 3. Synthwave Sunset & Grid (phase 0..1 for video or static)
  static void drawSynthwaveFrame(DisplayCanvas& gfx, float phase) {
    gfx.fillGradientV(0, 0, 128, 72, Color565::rgb(24, 6, 52), Color565::rgb(180, 30, 110));

    // Retro Striped Sun
    int16_t sunY = 44;
    gfx.fillCircle(64, sunY, 22, Color565::Gold);
    for (int y = sunY + 2; y < sunY + 22; y += 4) {
      gfx.fillRect(40, y, 48, 2, Color565::rgb(120, 20, 90));
    }

    // Distant Silhouette Mountains
    gfx.fillTriangle(0, 72, 28, 52, 56, 72, Color565::rgb(18, 10, 36));
    gfx.fillTriangle(44, 72, 68, 56, 92, 72, Color565::rgb(26, 12, 48));
    gfx.fillTriangle(76, 72, 104, 50, 128, 72, Color565::rgb(18, 10, 36));

    // Perspective Grid Floor
    gfx.fillRect(0, 72, 128, 56, Color565::rgb(12, 6, 26));
    gfx.drawFastHLine(0, 72, 128, Color565::NeonCyan);

    // Perspective rays from horizon center (64, 72)
    for (int x = -64; x <= 192; x += 24) {
      gfx.drawLine(64, 72, x, 127, Color565::Magenta);
    }

    // Moving horizontal grid bars
    for (int i = 0; i < 6; ++i) {
      float p = fmodf(i + phase, 6.0f) / 6.0f;
      int16_t gy = 72 + (int16_t)(p * p * 55.0f);
      if (gy > 72 && gy < 128) {
        gfx.drawFastHLine(0, gy, 128, Color565::NeonCyan);
      }
    }
  }

  // 4. Cute Pixel Art Cat
  static void drawPixelCat(DisplayCanvas& gfx) {
    gfx.fillGradientV(0, 0, 128, 128, Color565::rgb(20, 35, 70), Color565::rgb(45, 25, 65));
    // Stars & hearts
    gfx.fillCircle(22, 26, 2, Color565::Yellow);
    gfx.fillCircle(106, 30, 2, Color565::PinkHot);
    gfx.fillCircle(18, 92, 2, Color565::NeonCyan);

    // Cushion
    gfx.fillRoundRect(24, 92, 80, 22, 8, Color565::PinkHot);
    gfx.drawRoundRect(24, 92, 80, 22, 8, Color565::White);

    // Cat Ears
    gfx.fillTriangle(36, 52, 44, 28, 56, 52, Color565::Orange);
    gfx.fillTriangle(40, 50, 45, 34, 52, 50, Color565::PinkHot);
    gfx.fillTriangle(72, 52, 84, 28, 92, 52, Color565::Orange);
    gfx.fillTriangle(76, 50, 83, 34, 88, 50, Color565::PinkHot);

    // Cat Head & Body
    gfx.fillRoundRect(34, 46, 60, 46, 12, Color565::Gold);
    gfx.fillRoundRect(42, 74, 44, 24, 8, Color565::White); // White tummy

    // Eyes
    gfx.fillCircle(48, 64, 5, Color565::Black);
    gfx.fillCircle(46, 62, 2, Color565::White);
    gfx.fillCircle(80, 64, 5, Color565::Black);
    gfx.fillCircle(78, 62, 2, Color565::White);

    // Pink Nose & Whiskers
    gfx.fillTriangle(61, 68, 67, 68, 64, 72, Color565::PinkHot);
    gfx.drawLine(28, 66, 38, 68, Color565::White);
    gfx.drawLine(28, 72, 38, 72, Color565::White);
    gfx.drawLine(90, 68, 100, 66, Color565::White);
    gfx.drawLine(90, 72, 100, 72, Color565::White);

    // Paws
    gfx.fillCircle(48, 96, 6, Color565::White);
    gfx.fillCircle(80, 96, 6, Color565::White);

    gfx.drawCenteredText(10, "NEKO PIXEL", Color565::Yellow);
  }

public:
  static void seedIfNeeded(DisplayCanvas& gfx) {
    if (!LittleFS.begin(true)) return;

    if (!LittleFS.exists("/photos")) LittleFS.mkdir("/photos");
    if (!LittleFS.exists("/videos")) LittleFS.mkdir("/videos");

    bool needSeed = !LittleFS.exists("/photos/01_cyber_city.rgb") ||
                    !LittleFS.exists("/videos/01_astronaut_spin.mvid");

    if (!needSeed) return;

    // Show quick seeding message on screen
    gfx.clear(Color565::rgb(10, 15, 28));
    gfx.drawCenteredText(48, "INITIALIZING MEDIA", Color565::NeonCyan);
    gfx.drawCenteredText(64, "Creating Sample Photos", Color565::White);
    gfx.drawCenteredText(76, "& Videos in LittleFS...", Color565::Gold);
    gfx.present();

    // 1. Seed 4 Sample Photos (.rgb)
    if (!LittleFS.exists("/photos/01_cyber_city.rgb")) {
      drawCyberCity(gfx);
      saveCanvasAsRgb565(gfx, "/photos/01_cyber_city.rgb");
    }
    if (!LittleFS.exists("/photos/02_astronaut.rgb")) {
      drawAstronautFrame(gfx, 0.3f);
      gfx.drawCenteredText(6, "SPACE EXPLORER", Color565::Gold);
      saveCanvasAsRgb565(gfx, "/photos/02_astronaut.rgb");
    }
    if (!LittleFS.exists("/photos/03_synthwave.rgb")) {
      drawSynthwaveFrame(gfx, 0.2f);
      gfx.drawCenteredText(6, "OUTRUN 1986", Color565::NeonCyan);
      saveCanvasAsRgb565(gfx, "/photos/03_synthwave.rgb");
    }
    if (!LittleFS.exists("/photos/04_pixel_cat.rgb")) {
      drawPixelCat(gfx);
      saveCanvasAsRgb565(gfx, "/photos/04_pixel_cat.rgb");
    }

    // 2. Seed 2 Sample Multi-Frame Videos (.mvid, 12 frames each, 12 FPS, RGB332)
    const uint8_t frames = 12;
    const uint8_t hdr[8] = { 'M', 'V', '1', 1, 128, 128, 12, frames };

    if (!LittleFS.exists("/videos/01_astronaut_spin.mvid")) {
      File vf = LittleFS.open("/videos/01_astronaut_spin.mvid", "w");
      if (vf) {
        vf.write(hdr, 8);
        for (uint8_t i = 0; i < frames; ++i) {
          float ang = (i * 2.0f * 3.1415926f) / frames;
          drawAstronautFrame(gfx, ang);
          writeCanvasAsMvidFrame332(gfx, vf);
        }
        vf.close();
      }
    }

    if (!LittleFS.exists("/videos/02_synthwave_drive.mvid")) {
      File vf = LittleFS.open("/videos/02_synthwave_drive.mvid", "w");
      if (vf) {
        vf.write(hdr, 8);
        for (uint8_t i = 0; i < frames; ++i) {
          float ph = (float)i / (float)frames;
          drawSynthwaveFrame(gfx, ph);
          writeCanvasAsMvidFrame332(gfx, vf);
        }
        vf.close();
      }
    }
  }
};
