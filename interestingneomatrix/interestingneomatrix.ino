#define LED_PIN D1
// update at 50 FPS (realistic max with 1024 pixels is 30 anyways)
#define PERIOD_MS 20

#define SERIAL
#include "log.h"
#include "config.h"
#include "drawing.h"
#include "wifi.h"

void setup() {
  #ifdef SERIAL
  Serial.begin(115200);
  Serial.println("\n\n");
  #endif

  // attempting automatic reconnect in the background
  startWifi();

  loadConfig();
  startMatrix();
  setMsg();

  startWifiServices();
  LOG.println("Entering loop");
  delay(1000);
}

#define FPS_EVERY_MS 10000
uint32_t last_t;
uint16_t frames;

void loop() {
  uint32_t t = millis();
  if (t - last_t >= FPS_EVERY_MS) {
    ArduinoOTA.handle();

    fps = max((uint32_t) 1, (1000 * frames) / (t - last_t));
    LOG.printf("%ufps\n", fps);
    last_t = t;
    frames = 0;
  }
//  server.handleClient();
  draw(t);
  t = millis() - t;
  if (t < PERIOD_MS) {
    delay(PERIOD_MS - t);
  }
  frames++;
}
