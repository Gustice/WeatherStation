/**
 * @file Display_SSD1306.h
 * @author Gustice
 * @brief Display-Contoller implementation
 * @details This implementation is derived from "https://github.com/adafruit/Adafruit_SSD1306"
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once 

#include <stdint.h>
#include "Adafruit_GFX.h"
#include "SpiPort.h"
#include "GpioPort.h"

  // draw a single pixel
  // _display->drawPixel(10, 10, WHITE); 
  // draw many lines
  // _display->drawLine(0, 0, i, _display->height()-1, WHITE); 
  // draw rectangles
  // _display->drawRect(i, i, _display->width()-2*i, _display->height()-2*i, WHITE);
  // draw multiple rectangles
  // _display->fillRect(i, i, _display->width()-i*2, _display->height()-i*2, color%2);
  // draw mulitple circles
  // _display->drawCircle(_display->width()/2, _display->height()/2, i, WHITE);
  // draw a white circle, 10 pixel radius
  // _display->fillCircle(_display->width()/2, _display->height()/2, 10, WHITE);
  // draw rectangles with radius
  // _display->drawRoundRect(i, i, _display->width()-2*i, _display->height()-2*i, _display->height()/4, WHITE);
  // draw filled rectangles with radius
  // _display->fillRoundRect(i, i, _display->width()-2*i, _display->height()-2*i, _display->height()/4, color);
  // draw triangle
  // _display->drawTriangle(_display->width()/2, _display->height()/2-i,
                     // _display->width()/2-i, _display->height()/2+i,
                     // _display->width()/2+i, _display->height()/2+i, WHITE);
  // draw filled triangle 
  //   _display->fillTriangle(_display->width()/2, _display->height()/2-i,
                    //  _display->width()/2-i, _display->height()/2+i,
                    //  _display->width()/2+i, _display->height()/2+i, WHITE);
  // _display->setCursor(x,y); // Sets Anchor point (left,up) for next command
//   _display->startscrollright(0x00, 0x0F); // Scrolls content right
//   _display->startscrollleft(0x00, 0x0F); // Scrolls content left
//   _display->stopscroll(); // Stops scrolling
// _display->write(i); also prints invisible characters
//  static const unsigned char logo16_glcd_bmp[] = { 0x00, ... 0x30,  // Some continuously defined logo
// _draws Bitmap _display->drawBitmap(x, y, bitmap, w, h, WHITE); // Draws logo and
// _display->invertDisplay(true); // Inverts display

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 64

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

class SSD1306 : public Adafruit_GFX {
 public:
  SSD1306(SpiPort * spiI, OutputPort * dc, OutputPort * rst);
  
  void Init(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, bool reset=true);
  void ssd1306_command(uint8_t c);

  void clearDisplay(void);
  void invertDisplay(uint8_t i);
  void display();

  void startscrollright(uint8_t start, uint8_t stop);
  void startscrollleft(uint8_t start, uint8_t stop);

  void startscrolldiagright(uint8_t start, uint8_t stop);
  void startscrolldiagleft(uint8_t start, uint8_t stop);
  void stopscroll(void);

  void dim(bool dim);
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

 private:
  SpiPort * _spiI;
  OutputPort * _dc;
  OutputPort * _rst;
  int8_t _vccState;
  inline void drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color);
  inline void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color);
};












