#include <ArduinoOTA.h>

void startOTA() {
  ArduinoOTA.setHostname("everything");
  ArduinoOTA.setPassword((const char *) "foo");

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    byte percent = progress / (total / 100);
    if (percent % 10 == 0) {
      Serial.printf("Progress: %u%%\n", percent);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}
