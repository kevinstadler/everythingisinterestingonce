#define LED_PIN D1

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
  Serial.println("Setting message");
  setMsg();

  Serial.println("Starting services");
  startWifiServices();
  Serial.println("Entering loop");
  delay(1000);
}

// update at 50 FPS (realistic max is 30 anyways)
#define PERIOD_MS 20
#define FPS_EVERY_MS 1000
uint32_t last_t;
byte frames;

void loop() {
  uint32_t t = millis();
  if (t - last_t >= FPS_EVERY_MS) {
    fps = (FPS_EVERY_MS * frames) / (t - last_t);
//    Serial.printf("%ufps...", fps);
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
