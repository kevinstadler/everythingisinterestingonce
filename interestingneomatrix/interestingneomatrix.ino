#define LED_PIN D4

#include "config.h"
#include "drawing.h"
#include "wifi.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n");

  // attempting automatic reconnect in the background
  startWifi();

  loadConfig();
  startMatrix();
  setMsg();

  startWifiServices();
  Serial.println("Entering loop");
  delay(1000);
}

// update at 50 FPS (realistic max is 30 anyways)
#define PERIOD_MS 20
uint32_t last_t;
byte frames;

void loop() {
  uint32_t t = millis();
  if (t - last_t >= 1000) {
    Serial.printf("%u FPS\n", (1000 * frames) / (t - last_t));
    last_t = t;
    frames = 0;
    ArduinoOTA.handle();
  }
  server.handleClient();
  draw(t);
  t = millis() - t;
  if (t < PERIOD_MS) {
    delay(PERIOD_MS - t);
  }
  frames++;
}
