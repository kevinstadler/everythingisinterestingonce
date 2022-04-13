#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "font.h"

uint16_t distinctColors = 0;
uint32_t colors[1024];

uint16_t nLit = 0;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32*4, 8, LED_PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB);

#include "pixel.h"
// array of pointers
Pixel **pixels;

void startMatrix() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setFont(&Font5x7Fixed);

  // test all LEDs
  for (int16_t col = matrix.width()/8 - 1; col >= 0; col--) {
    matrix.fill((VALUE << 16) | (VALUE << 8) | VALUE, 8*col*matrix.height(), 8*matrix.height());
    matrix.show();
    delay(120);
    matrix.fill(0, 8*col*matrix.height(), 8*matrix.height());
    matrix.show();
  }
}

void draw(uint32_t t = millis()) {
  uint16_t changed = 0;
  for (uint16_t i = 0; i < nLit; i++) {
    uint32_t newCol = pixels[i]->getColor(t);
    if (newCol != matrix.getPixelColor(pixels[i]->index)) {
      matrix.setPixelColor(pixels[i]->index, newCol);
      changed++;
    }
  }
  if (changed > 0) {
//      Serial.printf("Redrawing because %u of %u pixels have changed\n", changed, nLit);
    matrix.show();
  }
}

uint16_t currentEstimate(bool gammaCorrect = CORRECT_GAMMA, byte value = VALUE) {
  uint32_t onePix = matrix.ColorHSV(0, SATURATION, value);
  if (gammaCorrect) {
    onePix = matrix.gamma32(onePix);
  }
  // 1mA baseline for every led (https://www.pjrc.com/how-much-current-do-ws2812-neopixel-leds-really-use/)
  // 20mA for any full on LED -- roughly scale from [0, 255] (of red LED) to [0, 19] by /13
  return matrix.width() * matrix.height() + nLit * (onePix >> 16) / 13;
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

  if (nLit > 0) {
    // TODO delete individual ones
    delete[] pixels;
  }
  nLit = 0;
  for (uint16_t i = 0; i < matrix.numPixels(); i++) {
    if (matrix.getPixelColor(i) != 0) {
      nLit++;
    }
  }

  Serial.printf("Message is %u pixels wide with %u pixels lit up, current draw %umA (%umA with gamma correct)\n",
    w, nLit, currentEstimate(false), currentEstimate(true));

  pixels = new Pixel*[nLit];

  PixelType type = Adding;
  for (uint16_t i = 0; i < nLit; i++) {
    switch (type) {
      case Synced:   pixels[i] = new SyncedPixel(); break;
      case Unsynced: pixels[i] = new UnsyncedPixel(); break;
      case Shifting: pixels[i] = new ShiftingPixel(); break;
      case Adding:   pixels[i] = new AddingPixel(); break;
    }
  }

  uint16_t j = 0;
  for (uint16_t i = 0; j < nLit; i++) {
    if (matrix.getPixelColor(i) != 0) {
      pixels[j]->index = i;
      j++;
    }
  }

  distinctColors = 1;
  colors[0] = matrix.ColorHSV(0, SATURATION, VALUE);

  for (uint16_t hue = 1; hue != 0; hue++) {
    colors[distinctColors] = matrix.ColorHSV(hue, SATURATION, VALUE);
    if (colors[distinctColors] != colors[distinctColors - 1]) {
      if (colors[distinctColors] == colors[0]) {
        // break if we're back at (initial) red
        break;
      }
      distinctColors++;
    }
  }
  Serial.printf("Randomly generating %u distinct hues, when the current LED brightness allows up to %u distinguishable hues\n", 1 << HUE_BITS, distinctColors);

  // how long does it take to enumerate all combos? -> INSANELY LONG
  // 260**300     use: x^a / x^b = x^(a-b)
//  double oneyear = 31536000.0;
  // 260**300 / oneyear = 260**somethingsmaller
  // change to logarithm of base lit
//  double yearExp = log(oneyear) / log(lit);
//  Serial.println(yearExp);
}
