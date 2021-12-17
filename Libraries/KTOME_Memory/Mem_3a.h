#include <Arduino.h>
#define mem3aWidth  40  // logo width
#define mem3aHeight 94  // logo height

const unsigned char no_3a[] PROGMEM = {
  0x00, 0xC0, 0xFF, 0x03, 0x00, 0x00, 0xF8, 0xFF, 0x0F, 0x00, 0x00, 0xFC, 
  0xFF, 0x3F, 0x00, 0x00, 0xFF, 0xFF, 0x7F, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 
  0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0xFF, 0x03, 0xE0, 
  0xFF, 0xFF, 0xFF, 0x07, 0xF0, 0xFF, 0xFF, 0xFF, 0x0F, 0xF0, 0xFF, 0xFF, 
  0xFF, 0x0F, 0xF8, 0xFF, 0xFF, 0xFF, 0x1F, 0xF8, 0xFF, 0xFF, 0xFF, 0x1F, 
  0xFC, 0xFF, 0x81, 0xFF, 0x3F, 0xFC, 0x7F, 0x00, 0xFF, 0x3F, 0xFC, 0x7F, 
  0x00, 0xFE, 0x3F, 0xFE, 0x3F, 0x00, 0xFE, 0x7F, 0xFE, 0x3F, 0x00, 0xFC, 
  0x7F, 0xFE, 0x1F, 0x00, 0xFC, 0x7F, 0xFE, 0x1F, 0x00, 0xF8, 0x7F, 0xFE, 
  0x1F, 0x00, 0xF8, 0xFF, 0xFF, 0x1F, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 
  0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 
  0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 
  0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 
  0xFF, 0xFF, 0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0xF8, 0xFF, 0xFF, 
  0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0xF8, 0x7F, 0x00, 0x00, 0x00, 
  0xFC, 0x7F, 0x00, 0x00, 0x00, 0xFC, 0x7F, 0x00, 0x00, 0x00, 0xFE, 0x7F, 
  0x00, 0x00, 0x00, 0xFE, 0x3F, 0x00, 0x00, 0x00, 0xFF, 0x3F, 0x00, 0x00, 
  0x80, 0xFF, 0x3F, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0xF8, 0xFF, 
  0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x07, 0x00, 
  0xF8, 0xFF, 0xFF, 0x03, 0x00, 0xF8, 0xFF, 0xFF, 0x03, 0x00, 0xF8, 0xFF, 
  0xFF, 0x01, 0x00, 0xF8, 0xFF, 0xFF, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x00, 
  0x00, 0xF8, 0xFF, 0xFF, 0x01, 0x00, 0xF8, 0xFF, 0xFF, 0x03, 0x00, 0xF8, 
  0xFF, 0xFF, 0x07, 0x00, 0xF8, 0xFF, 0xFF, 0x07, 0x00, 0xF8, 0xFF, 0xFF, 
  0x0F, 0x00, 0x00, 0xF0, 0xFF, 0x1F, 0x00, 0x00, 0xC0, 0xFF, 0x1F, 0x00, 
  0x00, 0x80, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0xFF, 0x3F, 0x00, 0x00, 0x00, 
  0xFE, 0x3F, 0x00, 0x00, 0x00, 0xFC, 0x7F, 0x00, 0x00, 0x00, 0xFC, 0x7F, 
  0x00, 0x00, 0x00, 0xF8, 0x7F, 0xFF, 0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 
  0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 
  0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 
  0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 
  0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0xF0, 0xFF, 
  0xFF, 0x0F, 0x00, 0xF8, 0xFF, 0xFF, 0x1F, 0x00, 0xF8, 0xFF, 0xFE, 0x1F, 
  0x00, 0xF8, 0x7F, 0xFE, 0x1F, 0x00, 0xF8, 0x7F, 0xFE, 0x1F, 0x00, 0xFC, 
  0x7F, 0xFE, 0x3F, 0x00, 0xFC, 0x7F, 0xFE, 0x3F, 0x00, 0xFC, 0x7F, 0xFC, 
  0x7F, 0x00, 0xFE, 0x3F, 0xFC, 0xFF, 0x00, 0xFF, 0x3F, 0xFC, 0xFF, 0xC1, 
  0xFF, 0x3F, 0xF8, 0xFF, 0xFF, 0xFF, 0x1F, 0xF8, 0xFF, 0xFF, 0xFF, 0x1F, 
  0xF0, 0xFF, 0xFF, 0xFF, 0x0F, 0xF0, 0xFF, 0xFF, 0xFF, 0x07, 0xE0, 0xFF, 
  0xFF, 0xFF, 0x07, 0xC0, 0xFF, 0xFF, 0xFF, 0x03, 0x80, 0xFF, 0xFF, 0xFF, 
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 
  0xFC, 0xFF, 0x1F, 0x00, 0x00, 0xF0, 0xFF, 0x07, 0x00, 0x00, 0x80, 0xFF, 
  0x00, 0x00, };
