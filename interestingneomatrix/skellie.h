const uint8_t Font5x7FixedBitmaps[] PROGMEM = {
  0x7D, 0xFE, 0x4F, 0xF7, 0xCA, 0x80, 0x75, 0x7E, 0xE5, 0x00, 0x7D, 0xFE, 
  0x4F, 0xF6, 0xC5, 0x00
};

const GFXglyph Font5x7FixedGlyphs[] PROGMEM = {
  {     0,   7,   6,   9,    1,   -5 },   // 0x2620 '☠'
  {     6,   5,   5,   7,    1,   -4 },   // 0x2622 '☢'
  {    10,   7,   6,   8,    0,   -5 }    // 0x2623 '☣'
};

const GFXfont Font5x7Fixed PROGMEM = {
  (uint8_t  *)Font5x7FixedBitmaps,         
  (GFXglyph *)Font5x7FixedGlyphs, 0x2620, 0x2623,          8};
