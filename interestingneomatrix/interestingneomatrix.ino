#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#define PIN D8

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, 6, 1, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };

void setup() {
  Serial.begin(115200);
  Serial.println();
  matrix.begin();
  matrix.setTextWrap(false);
//  matrix.setBrightness(40);
//  matrix.setTextColor(colors[0]);
  int16_t x1, y1;
  uint16_t w, h;
  // 180: everything is interesting once
  // 60: everything
  // 66: interesting
  matrix.getTextBounds("everything is interesting once", 0, 0, &x1, &y1, &w, &h);
  Serial.println(w);
  Serial.println(h);

  matrix.print("everything is interesting once");
  Serial.println(matrix.getCursorX());
  Serial.println(matrix.getCursorY());
  uint16_t lit = 0;
  for (uint16_t i = 0; i < matrix.numPixels(); i++) {
    if (matrix.getPixelColor(i) != 0) {
      lit++;
    }
  }
  Serial.println(lit);
}

void loop() {
  matrix.fillScreen(0);

}
