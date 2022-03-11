#define LED_PIN D7

#include "config.h"
#include "drawing.h"
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  Serial.println();
  startWifi();

  loadConfig();
  startMatrix();
  setMsg();

  startWifiServices();
}

void WAIT(uint32_t t = 10) {
  server.handleClient();
  delay(t);
}

// loop helper
uint32_t t;

void loop() {
  randomize();

  t = millis();
  if (FADE_IN) {
    // fade in -- linearly interpolate from 0 up to VALUE
    while (millis() < t + FADE_IN) {
      draw((VALUE * (millis() - t)) / FADE_IN);
      WAIT();
    }
  }

  draw();
  t += FADE_IN + (RANDOM_INTERVAL ? random(ON_MIN, ON_MAX) : ON_MIN);
  while (millis() < t) {
    WAIT();
  }

  if (FADE_OUT) {
    // fade out -- linearly interpolate from VALUE down to 0
    t += FADE_OUT;
    while (millis() < t) {
      draw((VALUE * (t - millis())) / FADE_OUT);
      WAIT();
    }
  }

  if (OFF_TIME) {
    draw(0);
    t += OFF_TIME;
    while (millis() < t) {
      WAIT();
    }
  }

  ArduinoOTA.handle();
}
