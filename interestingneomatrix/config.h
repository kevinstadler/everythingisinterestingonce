#include "LittleFS.h"
#define CONFIG_FILE "interesting.cfg"

// config parameters are written to file in this order
byte HUE_BITS = 5; // up to 16bit (65k hues), no point using more than 10bit (1024 hues)
byte SATURATION = 255;
byte VALUE = 31;
// actually distinguishable hues per brightness level (=HSV value):
// 63 -> 379
// 127 -> 763
// 255 -> 1531
bool CORRECT_GAMMA = false;
bool RANDOM_INTERVAL = false;

uint16_t ON_MIN = 1000;
uint16_t ON_MAX = 2000;
uint16_t OFF_TIME = 0;

String MSG = "everything is interesting once";

void loadConfig() {
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
  } else if (LittleFS.exists(CONFIG_FILE)) {
    Serial.println("Loading config file");
    File f = LittleFS.open(CONFIG_FILE, "r");
    HUE_BITS = f.read();
    SATURATION = f.read();
    VALUE = f.read();
    CORRECT_GAMMA = f.read();
    RANDOM_INTERVAL = f.read();
    ON_MIN = f.parseInt();
    ON_MAX = f.parseInt();
    OFF_TIME = f.parseInt();
    MSG = f.readString();
    f.close();
  }
}

void persistConfig() {
  Serial.println("Writing config file");
  File f = LittleFS.open(CONFIG_FILE, "w");
  f.write(HUE_BITS);
  f.write(SATURATION);
  f.write(VALUE);
  f.write((byte) CORRECT_GAMMA);
  f.write((byte) RANDOM_INTERVAL);
  f.println(ON_MIN);
  f.println(ON_MAX);
  f.println(OFF_TIME);
  f.println(MSG);
  f.close();
}
