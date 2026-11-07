#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "Config.h"

// Font5x7 data table
extern const uint8_t Font5x7[];

class DisplayCanvas {
private:
  uint16_t _buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
  SPIClass* _spi = nullptr;
  int8_t _shakeX = 0;
  int8_t _shakeY = 0;
  uint8_t _shakeFrames = 0;

  inline void writeCommand(uint8_t cmd) {
    digitalWrite(PIN_TFT_DC, LOW);
    digitalWrite(PIN_TFT_CS, LOW);
    _spi->transfer(cmd);
    digitalWrite(PIN_TFT_CS, HIGH);
  }

  inline void writeData(uint8_t data) {
    digitalWrite(PIN_TFT_DC, HIGH);
    digitalWrite(PIN_TFT_CS, LOW);
    _spi->transfer(data);
    digitalWrite(PIN_TFT_CS, HIGH);
  }

public:
  DisplayCanvas() {}

  void begin() {
    pinMode(PIN_TFT_CS, OUTPUT);
    pinMode(PIN_TFT_DC, OUTPUT);
    pinMode(PIN_TFT_RST, OUTPUT);
    digitalWrite(PIN_TFT_CS, HIGH);
    digitalWrite(PIN_TFT_DC, HIGH);

    // Setup Backlight PWM on ESP32-C3
    ledcAttach(PIN_TFT_BL, BL_PWM_FREQ, BL_PWM_RES);
    setBrightness(85);

    // Hardware Reset
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(10);
    digitalWrite(PIN_TFT_RST, LOW);
    delay(20);
    digitalWrite(PIN_TFT_RST, HIGH);
    delay(120);

    // Setup Hardware SPI (FSPI on ESP32-C3)
    _spi = new SPIClass(FSPI);
    _spi->begin(PIN_TFT_SCLK, -1, PIN_TFT_MOSI, PIN_TFT_CS);
    _spi->setFrequency(40000000); // 40 MHz high-speed SPI

    // ST7735 Initialization Sequence (GreenTab3 for 128x128)
    writeCommand(0x01); // SWRESET
    delay(150);
    writeCommand(0x11); // SLPOUT
    delay(200);

    writeCommand(0xB1); // FRMCTR1
    writeData(0x01); writeData(0x2C); writeData(0x2D);

    writeCommand(0xB2); // FRMCTR2
    writeData(0x01); writeData(0x2C); writeData(0x2D);

    writeCommand(0xB3); // FRMCTR3
    writeData(0x01); writeData(0x2C); writeData(0x2D);
    writeData(0x01); writeData(0x2C); writeData(0x2D);

    writeCommand(0xB4); // INVCTR
    writeData(0x07);

    writeCommand(0xC0); // PWCTR1
    writeData(0xA2); writeData(0x02); writeData(0x84);

    writeCommand(0xC1); // PWCTR2
    writeData(0xC5);

    writeCommand(0xC2); // PWCTR3
    writeData(0x0A); writeData(0x00);

    writeCommand(0xC3); // PWCTR4
    writeData(0x8A); writeData(0x2A);

    writeCommand(0xC4); // PWCTR5
    writeData(0x8A); writeData(0xEE);

    writeCommand(0xC5); // VMCTR1
    writeData(0x0E);

    writeCommand(0x20); // INVOFF (or 0x21 INVON based on panel)
    
    // MADCTL Memory Access Control (BGR, Rotation 2)
    writeCommand(0x36);
    writeData(0x00 | 0x08); // BGR color order

    writeCommand(0x3A); // COLMOD 16-bit color
    writeData(0x05);

    writeCommand(0x29); // DISPON
    delay(100);

    clear(Color565::Black);
    present();
  }

  void setBrightness(uint8_t percent) {
    if (percent > 100) percent = 100;
    uint32_t duty = (percent * 255) / 100;
    ledcWrite(PIN_TFT_BL, duty);
  }

  uint16_t* getBuffer() { return _buffer; }

  void clear(uint16_t color = Color565::Black) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
      _buffer[i] = color;
    }
  }

  void triggerShake(uint8_t frames = 6) {
    _shakeFrames = frames;
  }

  void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Offset for ST7735 128x128 Greentab3: colstart = 2, rowstart = 1
    uint8_t colOffset = 2;
    uint8_t rowOffset = 1;

    writeCommand(0x2A); // CASET
    writeData(0x00);
    writeData(x0 + colOffset);
    writeData(0x00);
    writeData(x1 + colOffset);

    writeCommand(0x2B); // RASET
    writeData(0x00);
    writeData(y0 + rowOffset);
    writeData(0x00);
    writeData(y1 + rowOffset);

    writeCommand(0x2C); // RAMWR
  }

  void present() {
    if (_shakeFrames > 0) {
      _shakeX = (random(0, 3) - 1);
      _shakeY = (random(0, 3) - 1);
      _shakeFrames--;
    } else {
      _shakeX = 0;
      _shakeY = 0;
    }

    setAddrWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

    digitalWrite(PIN_TFT_DC, HIGH);
    digitalWrite(PIN_TFT_CS, LOW);
    
    // Transfer entire 128x128 buffer at 40MHz via SPI (DMA / block transfer)
    _spi->writeBytes((uint8_t*)_buffer, sizeof(_buffer));
    digitalWrite(PIN_TFT_CS, HIGH);
  }

  inline void drawPixel(int16_t x, int16_t y, uint16_t color) {
    x += _shakeX;
    y += _shakeY;
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    _buffer[y * SCREEN_WIDTH + x] = color;
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (y + _shakeY < 0 || y + _shakeY >= SCREEN_HEIGHT) return;
    for (int16_t i = 0; i < w; ++i) {
      drawPixel(x + i, y, color);
    }
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (x + _shakeX < 0 || x + _shakeX >= SCREEN_WIDTH) return;
    for (int16_t i = 0; i < h; ++i) {
      drawPixel(x, y + i, color);
    }
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT || x + w <= 0 || y + h <= 0) return;
    int16_t x0 = max((int16_t)0, x);
    int16_t y0 = max((int16_t)0, y);
    int16_t x1 = min((int16_t)SCREEN_WIDTH, (int16_t)(x + w));
    int16_t y1 = min((int16_t)SCREEN_HEIGHT, (int16_t)(y + h));
    for (int16_t py = y0; py < y1; ++py) {
      for (int16_t px = x0; px < x1; ++px) {
        _buffer[(py + _shakeY) * SCREEN_WIDTH + (px + _shakeX)] = color;
      }
    }
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
  }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    fillRect(x + r, y, w - 2 * r, h, color);
    fillRect(x, y + r, r, h - 2 * r, color);
    fillRect(x + w - r, y + r, r, h - 2 * r, color);
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
  }

  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    drawFastHLine(x + r, y, w - 2 * r, color);
    drawFastHLine(x + r, y + h - 1, w - 2 * r, color);
    drawFastVLine(x, y + r, h - 2 * r, color);
    drawFastVLine(x + w - 1, y + r, h - 2 * r, color);
    drawCircleHelper(x + r, y + r, r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
  }

  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y) {
      if (f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 - x, y0 + y, color);
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 - x, y0 - y, color);
      drawPixel(x0 + y, y0 + x, color);
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 + y, y0 - x, color);
      drawPixel(x0 - y, y0 - x, color);
    }
  }

  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
  }

  void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
      if (f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      if (corners & 0x1) {
        drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
        drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
      }
      if (corners & 0x2) {
        drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
        drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
      }
    }
  }

  void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
      if (f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;
      if (cornername & 0x4) {
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 + y, y0 + x, color);
      }
      if (cornername & 0x2) {
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 + y, y0 - x, color);
      }
      if (cornername & 0x8) {
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 - x, y0 + y, color);
      }
      if (cornername & 0x1) {
        drawPixel(x0 - y, y0 - x, color);
        drawPixel(x0 - x, y0 - y, color);
      }
    }
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
      int16_t t = x0; x0 = y0; y0 = t;
      t = x1; x1 = y1; y1 = t;
    }
    if (x0 > x1) {
      int16_t t = x0; x0 = x1; x1 = t;
      t = y0; y0 = y1; y1 = t;
    }
    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++) {
      if (steep) drawPixel(y0, x0, color);
      else drawPixel(x0, y0, color);
      err -= dy;
      if (err < 0) {
        y0 += ystep;
        err += dx;
      }
    }
  }

  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    int16_t a, b, y, last;
    if (y0 > y1) {
      int16_t t = y0; y0 = y1; y1 = t;
      t = x0; x0 = x1; x1 = t;
    }
    if (y1 > y2) {
      int16_t t = y2; y2 = y1; y1 = t;
      t = x2; x2 = x1; x1 = t;
    }
    if (y0 > y1) {
      int16_t t = y0; y0 = y1; y1 = t;
      t = x0; x0 = x1; x1 = t;
    }

    if (y0 == y2) {
      a = b = x0;
      if (x1 < a) a = x1;
      else if (x1 > b) b = x1;
      if (x2 < a) a = x2;
      else if (x2 > b) b = x2;
      drawFastHLine(a, y0, b - a + 1, color);
      return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0;
    int16_t dx02 = x2 - x0, dy02 = y2 - y0;
    int16_t dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    if (y1 == y2) last = y1;
    else last = y1 - 1;

    for (y = y0; y <= last; y++) {
      a = x0 + sa / dy01;
      b = x0 + sb / dy02;
      sa += dx01;
      sb += dx02;
      if (a > b) { int16_t t = a; a = b; b = t; }
      drawFastHLine(a, y, b - a + 1, color);
    }

    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
      a = x1 + sa / dy12;
      b = x0 + sb / dy02;
      sa += dx12;
      sb += dx02;
      if (a > b) { int16_t t = a; a = b; b = t; }
      drawFastHLine(a, y, b - a + 1, color);
    }
  }

  void fillGradientV(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colTop, uint16_t colBot) {
    uint8_t r0 = (colTop >> 11) & 0x1F, g0 = (colTop >> 5) & 0x3F, b0 = colTop & 0x1F;
    uint8_t r1 = (colBot >> 11) & 0x1F, g1 = (colBot >> 5) & 0x3F, b1 = colBot & 0x1F;

    for (int16_t py = 0; py < h; ++py) {
      uint8_t r = r0 + ((r1 - r0) * py) / h;
      uint8_t g = g0 + ((g1 - g0) * py) / h;
      uint8_t b = b0 + ((b1 - b0) * py) / h;
      uint16_t c = (r << 11) | (g << 5) | b;
      drawFastHLine(x, y + py, w, c);
    }
  }

  // Draw 5x7 Character
  void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint8_t size = 1) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t* glyph = &Font5x7[(c - 32) * 5];

    for (uint8_t i = 0; i < 5; i++) {
      uint8_t line = pgm_read_byte(glyph + i);
      for (uint8_t j = 0; j < 8; j++) {
        if (line & 0x1) {
          if (size == 1) drawPixel(x + i, y + j, color);
          else fillRect(x + i * size, y + j * size, size, size, color);
        }
        line >>= 1;
      }
    }
  }

  void drawText(int16_t x, int16_t y, const char* str, uint16_t color, uint8_t size = 1) {
    int16_t curX = x;
    while (*str) {
      if (*str == '\n') {
        curX = x;
        y += 8 * size;
      } else {
        drawChar(curX, y, *str, color, size);
        curX += 6 * size;
      }
      str++;
    }
  }

  void drawCenteredText(int16_t y, const char* str, uint16_t color, uint8_t size = 1) {
    int len = strlen(str);
    int16_t w = len * 6 * size - size;
    int16_t x = (SCREEN_WIDTH - w) / 2;
    drawText(x, y, str, color, size);
  }

  void drawShadowText(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t shadowColor = Color565::Black, uint8_t size = 1) {
    drawText(x + 1, y + 1, str, shadowColor, size);
    drawText(x, y, str, color, size);
  }
};
