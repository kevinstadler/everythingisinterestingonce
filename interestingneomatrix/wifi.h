#include "server.h"
#include "ota.h"

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

WiFiEventHandler wifiConnectedHandler;
WiFiEventHandler gotIPHandler;

void onWifiConnected(const WiFiEventStationModeConnected& evt) {
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.SSID());
}

void onGotIP(const WiFiEventStationModeGotIP& evt) {
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
}

void startWifi() {
  wifiConnectedHandler = WiFi.onStationModeConnected(&onWifiConnected);
  gotIPHandler = WiFi.onStationModeGotIP(&onGotIP);

  // try auto-connecting to previously persisted wifi
  WiFi.begin();
}

void startWifiServices() {
  String wifiString = "";
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi not connected yet, searching for one using multi");
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
  
    // Register multi WiFi networks
    File networks = LittleFS.open(WIFI_FILE, "r");
    bool hasNetworks = false;
    if (networks) {
      Serial.println("Loading networks from file");
      while (networks.available()) {
        String w = networks.readStringUntil('\n');
        hasNetworks = true;
        w.trim();
        Serial.println(w);
        int16_t pos = w.indexOf(' ');
        if (pos == -1) {
          wifiMulti.addAP(w.c_str());
        } else {
          wifiMulti.addAP(w.substring(0, pos).c_str(), w.substring(pos+1).c_str());
        }
      }
    } else {
      Serial.println("Wifi config file not found, creating it");
      networks = LittleFS.open(WIFI_FILE, "w");
    }
    networks.close();
  
    if (!hasNetworks || wifiMulti.run(5000) != WL_CONNECTED) {
      Serial.println("No WiFi found, starting AP instead");
      // don't persist AP
      WiFi.persistent(false);
      if (WiFi.softAP("everythingisinterestingonce")) {
        wifiString = "everythingisinterestingonce: " + WiFi.softAPIP().toString();
      } else {
        wifiString = "total wifi failure";
      }
    } else {
      Serial.println("MultiWifi succeeded");
    }
  }
  if (wifiString.equals("")) {
    wifiString = WiFi.SSID() + ": " + WiFi.localIP().toString();
  }
  Serial.println(wifiString);

  matrix.clear();
  matrix.setCursor(0, 7);
  matrix.print(wifiString);
  matrix.show();
  delay(2000);
  matrix.clear();
  matrix.show();

  Serial.println("Starting Wifi services");
  startInterface();
  startOTA();
}
