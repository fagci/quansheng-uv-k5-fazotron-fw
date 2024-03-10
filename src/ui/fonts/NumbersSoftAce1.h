#include "../gfxfont.h"

const uint8_t SA_font1_Bitmaps[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0xC7, 0xFE, 0xE0, 0x7C, 0x03, 
  0xC0, 0x3C, 0x07, 0xC0, 0xFC, 0x1B, 0xC3, 0x3C, 0x23, 0xC4, 0x3C, 0xC3, 
  0xD8, 0x3F, 0x03, 0xE0, 0x3C, 0x03, 0xC0, 0x3E, 0x07, 0x7F, 0xE3, 0xFC, 
  0x0C, 0x73, 0xDF, 0xEF, 0x30, 0xC3, 0x0C, 0x30, 0xC3, 0x0C, 0x30, 0xC3, 
  0x0C, 0x30, 0xC3, 0x3F, 0xC7, 0xFE, 0xE0, 0x7C, 0x03, 0x00, 0x30, 0x03, 
  0x00, 0x30, 0x07, 0x00, 0xE0, 0x1C, 0x03, 0x80, 0x70, 0x0E, 0x01, 0xC0, 
  0x38, 0x07, 0x00, 0xE0, 0x0C, 0x00, 0xFF, 0xFF, 0xFF, 0x3F, 0xC7, 0xFE, 
  0xE0, 0x7C, 0x03, 0x00, 0x30, 0x03, 0x00, 0x30, 0x07, 0x0F, 0xE0, 0xFE, 
  0x00, 0x70, 0x03, 0x00, 0x30, 0x03, 0x00, 0x30, 0x03, 0xC0, 0x3E, 0x07, 
  0x7F, 0xE3, 0xFC, 0x00, 0xC0, 0x18, 0x07, 0x00, 0xE0, 0x3C, 0x07, 0x81, 
  0xB0, 0x36, 0x0C, 0xC1, 0x98, 0x63, 0x0C, 0x63, 0x0C, 0x61, 0x98, 0x33, 
  0x06, 0xFF, 0xFF, 0xFC, 0x03, 0x00, 0x60, 0xFF, 0xFF, 0xFF, 0xC0, 0x0C, 
  0x00, 0xC0, 0x0C, 0x00, 0xC0, 0x0C, 0x00, 0xFF, 0xCF, 0xFE, 0xE0, 0x7C, 
  0x03, 0x00, 0x30, 0x03, 0x00, 0x30, 0x03, 0xC0, 0x3E, 0x07, 0x7F, 0xE3, 
  0xFC, 0x3F, 0xC7, 0xFE, 0xE0, 0x7C, 0x03, 0xC0, 0x0C, 0x00, 0xC0, 0x0C, 
  0x00, 0xFF, 0xCF, 0xFE, 0xE0, 0x7C, 0x03, 0xC0, 0x3C, 0x03, 0xC0, 0x3C, 
  0x03, 0xC0, 0x3E, 0x07, 0x7F, 0xE3, 0xFC, 0xFF, 0xFF, 0xF0, 0x0C, 0x03, 
  0x01, 0x80, 0x60, 0x30, 0x0C, 0x06, 0x01, 0x80, 0xC0, 0x30, 0x18, 0x06, 
  0x03, 0x00, 0xC0, 0x60, 0x18, 0x06, 0x01, 0x80, 0x3F, 0xC7, 0xFE, 0xE0, 
  0x7C, 0x03, 0xC0, 0x3C, 0x03, 0xC0, 0x3E, 0x07, 0x7F, 0xE7, 0xFE, 0xE0, 
  0x7C, 0x03, 0xC0, 0x3C, 0x03, 0xC0, 0x3C, 0x03, 0xC0, 0x3E, 0x07, 0x7F, 
  0xE3, 0xFC, 0x3F, 0xC7, 0xFE, 0xE0, 0x7C, 0x03, 0xC0, 0x3C, 0x03, 0xC0, 
  0x3E, 0x07, 0x7F, 0xF3, 0xFF, 0x00, 0x30, 0x03, 0x00, 0x30, 0x03, 0x00, 
  0x30, 0x03, 0xC0, 0x3E, 0x07, 0x7F, 0xE3, 0xFC
};

const GFXglyph SA_font1_Glyphs[] PROGMEM = {
  {     0,   5,  20,   6,    0,  -19 },   // 0x2D '-'
  {    13,   2,  20,   3,    0,  -19 },   // 0x2E '.'
  {     0,   0,   0,   0,    0,    0 },   // 0x2F '/'
  {    18,  12,  20,  13,    0,  -19 },   // 0x30 '0'
  {    48,   6,  20,   7,    0,  -19 },   // 0x31 '1'
  {    63,  12,  20,  13,    0,  -19 },   // 0x32 '2'
  {    93,  12,  20,  13,    0,  -19 },   // 0x33 '3'
  {   123,  11,  20,  12,    0,  -19 },   // 0x34 '4'
  {   151,  12,  20,  13,    0,  -19 },   // 0x35 '5'
  {   181,  12,  20,  13,    0,  -19 },   // 0x36 '6'
  {   211,  10,  20,  11,    0,  -19 },   // 0x37 '7'
  {   236,  12,  20,  13,    0,  -19 },   // 0x38 '8'
  {   266,  12,  20,  13,    0,  -19 }    // 0x39 '9'
};

const GFXfont SA_font1 PROGMEM = {(uint8_t *) SA_font1_Bitmaps,   (GFXglyph *)SA_font1_Glyphs, 0x2D, 0x39,   21};
