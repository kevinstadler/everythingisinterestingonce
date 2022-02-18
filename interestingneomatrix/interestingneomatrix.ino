#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "font.h"

#define LED_PIN D7

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(64, 8, LED_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB);

#define MSG "everything is interesting once"
uint16_t lit = 0;
uint16_t *litPixels;

#define BRIGHTNESS 63
// actually distinguishable hues per brightness level (=HSV value):
// 63 -> 379
// 127 -> 763
// 255 -> 1531

#define HUE_BITS 10 // up to 16bit (65k hues), let's just use 10 (512 hues)

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.setTimeout(50);
  
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setFont(&Font5x7Fixed);
  int16_t x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(MSG, 0, 0, &x1, &y1, &w, &h);
  Serial.println(w);
  Serial.println(h);

  uint16_t indent = (matrix.width() - matrix.getCursorX()); // TODO compute
  indent = 1;
  matrix.setCursor(indent, 7);

  // draw and count, then index filled pixels
  matrix.print(MSG);
  for (uint16_t i = 0; i < matrix.numPixels(); i++) {
    if (matrix.getPixelColor(i) != 0) {
      lit++;
    }
  }
  Serial.println(lit);
  litPixels = new uint16_t[lit];
  uint16_t j = 0;
  for (uint16_t i = 0; j < lit; i++) {
    if (matrix.getPixelColor(i) != 0) {
      litPixels[j] = i;
      j++;
    }
  }

  uint16_t distinctCols = 1;
  uint32_t lastCol = matrix.ColorHSV(0, 255, BRIGHTNESS);
  uint32_t col;
  for (uint16_t hue = 1; hue != 0; hue++) {
    col = matrix.ColorHSV(hue, 255, BRIGHTNESS);
    if (col != lastCol) {
      lastCol = col;
      distinctCols++;
    }
  }
  Serial.print("Number of distinguishable colours at this brightness: ");
  Serial.println(distinctCols);
}

uint16_t blnk = 1500;

void loop() {
  for (int i = 0; i < lit; i++) {
    // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
    uint32_t col = matrix.ColorHSV(random() << (16 - HUE_BITS), 255, BRIGHTNESS);
    matrix.setPixelColor(litPixels[i], col); // skip matrix.gamma32() here
  }
  matrix.show();
  delay(blnk);

  matrix.fillScreen(0);
  matrix.show();
  delay(50);

  uint16_t input = Serial.parseInt();
  if (input != 0) {
    blnk = input;
  }
}
