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
byte fps = 28;
#define LED_TYPE NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG

#define N_PANELS 4

#ifdef FASTLED
CRGB leds[32 * 8 * N_PANELS];
FastLED_NeoMatrix matrix = FastLED_NeoMatrix(leds, 32 * N_PANELS, 8, LED_TYPE);
#else
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32 * N_PANELS, 8, LED_PIN, LED_TYPE, NEO_GRB);
#endif

#include "pixel.h"
// array of pointers
Pixel **pixels;

uint16_t distinctColors = 0;
CRGB colors[1024];

void startMatrix() {
#ifdef FASTLED
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, matrix.width() * matrix.height()); //.setCorrection(TypicalLEDStrip);
#endif

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setFont(&Font5x7Fixed);

  // test all LEDs
  for (int16_t col = matrix.width() / 8 - 1; col >= 0; col--) {
#ifdef FASTLED
    for (byte i = 0; i < 8 * matrix.height(); i++) {
      //((CONFIG.VALUE << 11) | (CONFIG.VALUE << 6) | CONFIG.VALUE
      leds[8 * col * matrix.height() + i].r = CONFIG.VALUE;
      leds[8 * col * matrix.height() + i].g = CONFIG.VALUE;
      leds[8 * col * matrix.height() + i].b = CONFIG.VALUE;
    }
#else
    matrix.fill((CONFIG.VALUE << 16) | (CONFIG.VALUE << 8) | CONFIG.VALUE, 8 * col * matrix.height(), 8 * matrix.height());
#endif

    matrix.show();
    delay(120);
    matrix.clear();
  }
  matrix.show();
}

void showMsg(String msg) {
  matrix.clear();
  matrix.setCursor(0, 7);
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
    FastLED.delay(5);
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

  Serial.printf("Message is %u pixels wide with %u pixels lit up, current draw %umA (%umA with gamma correct)\n",
                w, nLit, currentEstimate(false), currentEstimate(true));

  pixels = new Pixel*[nLit];

  for (uint16_t i = 0; i < nLit; i++) {
    // can't seen to call random() in the pixel constructor, so just abuse the
    /// transition probability variable which will anyway be recalculated below
//    transitionP = random();
    pixels[i] = new Pixel();
/*    switch (CONFIG.ANIMATION[0].PIXEL_TYPE) {
      case Synced:   pixels[i] = new SyncedPixel(); break;
      case Unsynced: pixels[i] = new UnsyncedPixel(); break;
      case Radial:   pixels[i] = new RadialBlendingPixel(); break;
      case Linear:   pixels[i] = new LinearBlendingPixel(); break;
      case Flicker:   pixels[i] = new FlickeringPixel(); break;
    }*/
    pixels[i]->init();
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
  Serial.printf("Randomly generating %u distinct hues, when the current LED brightness allows up to %u distinguishable hues\n", 1 << CONFIG.HUE_BITS, distinctColors);

  // calculate transition probability
  uint16_t meanTimeBetween = (CONFIG.ANIMATION[1].ON_TIME + CONFIG.ANIMATION[1].ON_EXTRA/2 + CONFIG.ANIMATION[1].TRANSITION_DURATION) / nLit;
  if (CONFIG.ANIMATION[0].LIMIT_CHANGES <= meanTimeBetween) {
    transitionPs[0] = PROBABILITY_BASE;
    Serial.println("Transition will be immediate");
  } else {
    // if meantime is 100 and limit is 200 -> 1 of the nLit pixels should trigger after 100 ms
    // if meantime is 100 and limit is 1000 -> 1 of the nLit should trigger after 900ms
    // per-pixel chance of triggering: probability base = X * fps / (1000 * (limit_changes - meantime))
    transitionPs[0] = PROBABILITY_BASE * 1000 / (nLit * fps * (CONFIG.ANIMATION[0].LIMIT_CHANGES - meanTimeBetween));
    Serial.printf("Transition will be stochastic, roughly 1 in %u\n", PROBABILITY_BASE / transitionPs[0]);
//    Serial.println(matrix.fps());
  }

  // how long does it take to enumerate all combos? -> INSANELY LONG
  // 260**300     use: x^a / x^b = x^(a-b)
  //  double oneyear = 31536000.0;
  // 260**300 / oneyear = 260**somethingsmaller
  // change to logarithm of base lit
  //  double yearExp = log(oneyear) / log(lit);
  //  Serial.println(yearExp);
}
