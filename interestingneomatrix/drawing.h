#define FASTLED

#ifdef FASTLED
// FastLED software SPI: 108FPS for one panel, 28FPS for 4
#define FASTLED_ALL_PINS_HARDWARE_SPI
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#else
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define CHSV(H, S, V) matrix.ColorHSV(H, S, V)
#define CRGB uint32_t
#endif

#include "font.h"
uint16_t nLit = 0;
byte fps = 14;//1000 / PERIOD_MS;
#define LED_TYPE NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG

#define N_PANELS 4
#define LED_WIDTH 32 * N_PANELS

#ifdef FASTLED
CRGB leds[8 * LED_WIDTH];
FastLED_NeoMatrix matrix = FastLED_NeoMatrix(leds, LED_WIDTH, 8, LED_TYPE);
#else
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32 * N_PANELS, 8, LED_PIN, LED_TYPE, NEO_GRB);
#endif

#include "pixel.h"
// array of pointers
Pixel **pixels;

uint16_t distinctColors = 0;
CRGB colors[1024];

void fillRandomColors(uint8_t width) {
  for (uint16_t i = 0; i < width * matrix.height(); i++) {
    leds[matrix.width()*matrix.height() - 1 - i] = CHSV(random(), CONFIG.SATURATION, CONFIG.VALUE);
  }
}

void startMatrix() {
#ifdef FASTLED
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, matrix.width() * matrix.height()); //.setCorrection(TypicalLEDStrip);
#endif

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setFont(&Font5x7Fixed);

  // test all LEDs
/*  for (int16_t col = matrix.width() / 8 - 1; col >= 0; col--) {
#ifdef FASTLED
    for (byte i = 0; i < 8 * matrix.height(); i++) {
      leds[8 * col * matrix.height() + i] = CRGB(CONFIG.VALUE, CONFIG.VALUE, CONFIG.VALUE);
//      leds[8 * col * matrix.height() + i] = CHSV(random(), CONFIG.SATURATION, CONFIG.VALUE);
    }
#else
    matrix.fill((CONFIG.VALUE << 16) | (CONFIG.VALUE << 8) | CONFIG.VALUE, 8 * col * matrix.height(), 8 * matrix.height());
#endif

    matrix.show();
    delay(100);
    matrix.clear();
  }*/
  for (uint8_t i = 1; i <= matrix.width(); i++) {
    fillRandomColors(i);
    matrix.show();
  }
  matrix.clear();
  matrix.show();
}

void showMsg(String msg) {
  matrix.clear();
  matrix.setCursor(0, 6);
  matrix.setPassThruColor(CRGB(CONFIG.VALUE/3, CONFIG.VALUE/3, CONFIG.VALUE/3));
  matrix.print(msg);
  matrix.show();
  matrix.setPassThruColor();
  FastLED.delay(5);
  delay(2000);
  matrix.clear();
  matrix.show();
}

void draw(uint32_t t = millis()) {
  uint16_t changed = 0;
  for (uint16_t i = 0; i < nLit; i++) {
    CRGB newCol = pixels[i]->getColor(t);
#ifdef FASTLED
    if (newCol != leds[pixels[i]->index]) {
      leds[pixels[i]->index] = newCol;
      changed++;
    }
#else
    if (newCol != matrix.getPixelColor(pixels[i]->index)) {
      leds[pixels[i]->index] = newCol;
      matrix.setPixelColor(pixels[i]->index, newCol);
      changed++;
    }
#endif
  }
  if (changed > 0) {
    //      Serial.printf("Redrawing because %u of %u pixels have changed\n", changed, nLit);
    matrix.show();
    // for some reason this is necessary to make sure reads go out
    FastLED.delay(1);
  }
}

uint16_t currentEstimate(bool gammaCorrect = CONFIG.CORRECT_GAMMA, byte value = CONFIG.VALUE) {
#ifdef FASTLED
  byte onePx = CHSV(0, CONFIG.SATURATION, value)[0];
#else
  uint32_t onePx = matrix.ColorHSV(0, CONFIG.SATURATION, value);
  if (gammaCorrect) {
    onePx = matrix.gamma32(onePx);
  }
  onePx => > 16;
#endif
  // 1mA baseline for every led (https://www.pjrc.com/how-much-current-do-ws2812-neopixel-leds-really-use/)
  // 20mA for any full on LED -- roughly scale from [0, 255] (of red LED) to [0, 19] by /13
  return matrix.width() * matrix.height() + nLit * onePx / 13;
}

void setMsg(String msg = MSG) {
  msg.trim();
  if (msg.length() == 0) {
    return;
  }
  MSG = msg;
  int16_t x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(MSG, 0, 0, &x1, &y1, &w, &h);
  uint16_t indent = (matrix.width() - w) / 2;
  matrix.setCursor(indent, 6);

  // draw sentence and count then index filled pixels
  matrix.fillScreen(0);
  matrix.print(MSG);

  if (nLit > 0) {
    for (uint16_t i = 0; i < nLit; i++) {
      delete pixels[i];
    }
    delete[] pixels;
  }
  nLit = 0;
  for (uint16_t i = 0; i < (matrix.width() * matrix.height()); i++) {
    //    if (matrix.getPixelColor(i) != 0) {
    if (leds[i].b != 0) {
      nLit++;
    }
  }

  LOG.printf("Message is %u pixels wide with %u pixels lit up, current draw %umA (%umA with gamma correct)\n",
                w, nLit, currentEstimate(false), currentEstimate(true));

  pixels = new Pixel*[nLit];
  for (uint16_t i = 0; i < nLit; i++) {
    pixels[i] = new Pixel();
  }

  uint16_t j = 0;
  for (uint16_t i = 0; j < nLit; i++) {
#ifdef FASTLED
    if (leds[i].r != 0) {
#else
    if (matrix.getPixelColor(i) != 0) {
#endif
      pixels[j]->index = i;
      j++;
    }
  }

  distinctColors = 1;
  colors[0] = CHSV(0, CONFIG.SATURATION, CONFIG.VALUE);

  for (uint16_t hue = 1; hue != 0; hue++) {
    colors[distinctColors] = CHSV(hue, CONFIG.SATURATION, CONFIG.VALUE);
    if (colors[distinctColors] != colors[distinctColors - 1]) {
      if (colors[distinctColors] == colors[0]) {
        // break if we're back at (initial) red
        break;
      }
      distinctColors++;
    }
  }
  LOG.printf("Randomly generating %u distinct hues, when the current LED brightness allows up to %u distinguishable hues\n", 1 << CONFIG.HUE_BITS, distinctColors);

  // calculate transition probability
  uint16_t meanTimeBetween = CONFIG.ANIMATION[1].TRANSITION_DURATION + CONFIG.ANIMATION[1].TRANSITION_EXTRA/2;
  if (CONFIG.ANIMATION[1].PACE_TRANSITIONS) {
    meanTimeBetween /= 2;
  }
  meanTimeBetween += CONFIG.ANIMATION[1].ON_TIME + CONFIG.ANIMATION[1].ON_EXTRA/2;
  meanTimeBetween /= nLit;
  meanTimeBetween = max((uint16_t) 1, meanTimeBetween);

  if (CONFIG.ANIMATION[0].LIMIT_CHANGES <= meanTimeBetween) {
    transitionPs[0] = PROBABILITY_BASE;
    LOG.println("Transition will be immediate");
  } else {
    // if meantime is 100 and limit is 200 -> 1 of the nLit pixels should trigger after 100 ms
    // if meantime is 100 and limit is 1000 -> 1 of the nLit should trigger after 900ms
    // we want expected value of the binomial (number of successes) = n*p = 1 / ( (limit_changes-meanTimeBetween) * fps)
    // per-pixel chance of triggering: probability base = X * fps / (1000 * (limit_changes - meantime))
    LOG.printf("Mean time between ready pixels is %ums, which is less than the limit of %ums\n", meanTimeBetween, CONFIG.ANIMATION[0].LIMIT_CHANGES);

    // easier: reduce probability to 1 in CONFIG.ANIMATION[0].LIMIT_CHANGES / meanTimeBetween
    transitionPs[0] = PROBABILITY_BASE * meanTimeBetween / CONFIG.ANIMATION[0].LIMIT_CHANGES;
//    transitionPs[0] = PROBABILITY_BASE * 1000;
//    transitionPs[0] /= saveFps;
//    transitionPs[0] /= nLit;
//    transitionPs[0] /= CONFIG.ANIMATION[0].LIMIT_CHANGES - meanTimeBetween;
    LOG.printf("Transition will be stochastic, probability of triggering per pixel per (%ums) frame = %.4f\n", 1000/fps, transitionPs[0] / (float) PROBABILITY_BASE);
  }

  // how long does it take to enumerate all combos? -> INSANELY LONG
  // 260**300     use: x^a / x^b = x^(a-b)
  //  double oneyear = 31536000.0;
  // 260**300 / oneyear = 260**somethingsmaller
  // change to logarithm of base lit
  //  double yearExp = log(oneyear) / log(lit);
  //  Serial.println(yearExp);
}
