#include "server.h"
#include "ota.h"

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

WiFiEventHandler wifiConnectedHandler;
WiFiEventHandler gotIPHandler;

volatile bool validConnection = false;

void onWifiConnected(const WiFiEventStationModeConnected& evt) {
  LOG.print("WiFi connected: ");
  LOG.println(WiFi.SSID());
  // TODO check against file
  //if (WiFi.SSID().equals(...)) {
//    WiFi.disconnect();
//  }
}

void onGotIP(const WiFiEventStationModeGotIP& evt) {
  LOG.print("Got IP: ");
  LOG.println(WiFi.localIP());
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
    LOG.println("Wifi not connected yet, searching for one using multi");
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
  
    // Register multi WiFi networks
    File networks = LittleFS.open(WIFI_FILE, "r");
    bool hasNetworks = false;
    wifiMulti.addAP("dk_sozialraum", "doyourdishes");
    hasNetworks = true;
    if (networks) {
      LOG.println("Loading networks from file");
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
      LOG.println("Wifi config file not found, creating it");
      networks = LittleFS.open(WIFI_FILE, "w");
    }
    networks.close();
  
    if (!hasNetworks || wifiMulti.run(5000) != WL_CONNECTED) {
      LOG.println("No WiFi found, starting AP instead");
      // don't persist AP
      WiFi.persistent(false);
      if (WiFi.softAP("everythingisinterestingonce")) {
        wifiString = "everythingisinterestingonce: " + WiFi.softAPIP().toString();
      } else {
        wifiString = "total wifi failure";
      }
    } else {
      LOG.println("MultiWifi succeeded");
    }
  }
  if (wifiString.equals("")) {
    wifiString = WiFi.SSID() + ": " + WiFi.localIP().toString();
  }
  LOG.println(wifiString);
  showMsg(wifiString);

  #ifdef SERIAL
  LOG.println("Starting Wifi services");
  #else
  TelnetStream.begin();
  #endif
  startInterface();
  startOTA();
}
