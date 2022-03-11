#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "font.h"

uint16_t distinctColors = 0;
uint32_t colors[1024];

uint16_t lit = 0;
uint16_t *litPixels;
uint16_t *hues;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(128, 8, LED_PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB);

void startMatrix() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setFont(&Font5x7Fixed);

  // test all LEDs
  for (int16_t col = matrix.width() - 1; col >= 0; col--) {
    matrix.fill((VALUE << 16) | (VALUE << 8) | VALUE, col*matrix.height(), matrix.height());
    matrix.show();
    delay(5);
    matrix.fill(0, col*matrix.height(), matrix.height());
  }
}

void randomize() {
  for (uint16_t i = 0; i < lit; i++) {
    hues[i] = random();
  }
}

void draw(byte brightness = VALUE) {
  if (brightness == 0) {
    matrix.fillScreen(0);
  } else {
    for (uint16_t i = 0; i < lit; i++) {
      // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
      uint32_t col = matrix.ColorHSV(hues[i] << (16 - HUE_BITS), SATURATION, brightness);
      matrix.setPixelColor(litPixels[i], CORRECT_GAMMA ? matrix.gamma32(col) : col);
    }
  }
  matrix.show();
}

uint16_t currentEstimate(bool gammaCorrect = CORRECT_GAMMA, byte value = VALUE) {
  uint32_t onePix = matrix.ColorHSV(0, SATURATION, value);
  if (gammaCorrect) {
    onePix = matrix.gamma32(onePix);
  }
  // 1mA baseline for every led (https://www.pjrc.com/how-much-current-do-ws2812-neopixel-leds-really-use/)
  // 20mA for any full on LED -- roughly scale from [0, 255] (of red LED) to [0, 19] by /13
  return matrix.width() * matrix.height() + lit * (onePix >> 16) / 13;
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

  Serial.printf("Message is %u pixels wide with %u pixels lit up, current draw %umA (%umA with gamma correct)\n",
    w, lit, currentEstimate(false), currentEstimate(true));

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
  Serial.printf("Up to %u distinguishable colors at this brightness\n", distinctColors);
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
