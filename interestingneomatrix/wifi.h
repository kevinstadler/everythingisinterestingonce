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

  WiFi.begin();
}

void startWifiServices() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi not connected yet, searching for one using multi");
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
  
    // Register multi WiFi networks
  
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("No WiFi found, starting AP instead");
      // don't persist AP
      WiFi.persistent(false);
      Serial.println(WiFi.softAP("everythingisinterestingonce") ? "Ready" : "Failed!");
      Serial.print("Soft-AP IP address = ");
      Serial.println(WiFi.softAPIP());
    }
  }

  Serial.println("Starting Wifi services");
  startInterface();
  startOTA();
}
