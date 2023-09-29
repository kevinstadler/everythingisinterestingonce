#include <ArduinoOTA.h>

void startOTA() {
  ArduinoOTA.setHostname("everything");
  ArduinoOTA.setPassword((const char *) "foo");

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    byte percent = progress / (total / 100);
    uint16_t pixels = LED_WIDTH * progress / total;
/*    if (percent % 10 == 0) {
      LOG.printf("...%u%%, %u/%u", percent, pixels, LED_WIDTH);
      showMsg(String(progress) + "/" + String(total));
    }*/
    LOG.printf("%u/%u\n", progress, total);
//    leds[0].r = CONFIG.VALUE;
    for (uint16_t i = 0; i < pixels * matrix.height(); i++) {
      leds[matrix.width()*matrix.height() - 1 - i] = CHSV(random(), CONFIG.SATURATION, CONFIG.VALUE);
//      leds[i].r = leds[0].r;
//      leds[i].g = leds[0].g;
//      leds[i].b = leds[0].b;
    }
    matrix.show();
    matrix.show();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    LOG.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      LOG.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      LOG.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      LOG.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      LOG.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      LOG.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  // for good measure (before anything else breaks)
  ArduinoOTA.handle();
}
