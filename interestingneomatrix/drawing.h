#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "font.h"

uint16_t lit = 0;
uint16_t *litPixels;
uint16_t *hues;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(128, 8, LED_PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB);

void randomize() {
  for (uint16_t i = 0; i < lit; i++) {
    hues[i] = random();
  }
}

void draw() {
  for (uint16_t i = 0; i < lit; i++) {
    // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
    uint32_t col = matrix.ColorHSV(hues[i] << (16 - HUE_BITS), SATURATION, VALUE);
    matrix.setPixelColor(litPixels[i], CORRECT_GAMMA ? matrix.gamma32(col) : col);
  }
  matrix.show();
}

void setMsg(String msg = MSG) {
  MSG = msg;
  MSG.trim();
  int16_t x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(MSG, 0, 0, &x1, &y1, &w, &h);
  uint16_t indent = (matrix.width() - w) / 2;
  matrix.setCursor(indent, 7);

  // draw sentence and count then index filled pixels
  matrix.fillScreen(0);
  matrix.print(MSG);

  if (lit > 0) {
    delete[] litPixels;
    delete[] hues;
  }
  lit = 0;
  for (uint16_t i = 0; i < matrix.numPixels(); i++) {
    if (matrix.getPixelColor(i) != 0) {
      lit++;
    }
  }

  Serial.print("Message is ");
  Serial.print(w);
  Serial.print(" pixels wide with ");
  Serial.print(lit);
  Serial.println(" pixels lit up.");

  litPixels = new uint16_t[lit];
  hues = new uint16_t[lit];
  uint16_t j = 0;
  for (uint16_t i = 0; j < lit; i++) {
    if (matrix.getPixelColor(i) != 0) {
      litPixels[j] = i;
      j++;
    }
  }
  // fill them hues
  randomize();

  uint16_t distinctCols = 1;
  uint32_t lastCol = matrix.ColorHSV(0, SATURATION, VALUE);

  uint16_t distinctColsGamma = 1;
  uint32_t lastColGamma = matrix.gamma32(lastCol);

  uint32_t col, colGamma;
  for (uint16_t hue = 1; hue != 0; hue++) {
    col = matrix.ColorHSV(hue, SATURATION, VALUE);
    if (col != lastCol) {
      lastCol = col;
      distinctCols++;
      colGamma = matrix.gamma32(lastCol);
      if (colGamma != lastColGamma) {
        lastColGamma = colGamma;
        distinctColsGamma++;
      }
    }
  }
  Serial.print("Number of distinguishable colors at this brightness: ");
  Serial.print(distinctCols);
  Serial.print(" (");
  Serial.print(distinctColsGamma);
  Serial.println(" with gamma correction)");
  Serial.print("Number of distinct hues that will be randomly generated: ");
  Serial.println(1 << HUE_BITS);

  // how long does it take to enumerate all combos? -> INSANELY LONG
  // 260**300     use: x^a / x^b = x^(a-b)
//  double oneyear = 31536000.0;
  // 260**300 / oneyear = 260**somethingsmaller
  // change to logarithm of base lit
//  double yearExp = log(oneyear) / log(lit);
//  Serial.println(yearExp);
}
