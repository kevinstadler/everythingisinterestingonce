#define LED_PIN D7

#include "config.h"
#include "drawing.h"
#include "server.h"

void setup() {
  Serial.begin(115200);
  Serial.println();
//  Serial.setTimeout(50);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setFont(&Font5x7Fixed);

  startInterface();
  loadConfig();
  Serial.println(MSG);
  setMsg();
  Serial.println(MSG);
}

void loop() {
  randomize();
  draw();
  uint32_t until = millis() + (RANDOM_INTERVAL ? random(ON_MIN, ON_MAX) : ON_MIN);
  Serial.print(until - millis());
  while (millis() < until) {
    server.handleClient();
    delay(5);
  }

  if (OFF_TIME) {
    until = millis() + OFF_TIME;
    Serial.print('/');
    Serial.print(until - millis());
    matrix.fillScreen(0);
    matrix.show();
    while (millis() < until) {
      server.handleClient();
      delay(5);
    }
  }
  Serial.println();
}
