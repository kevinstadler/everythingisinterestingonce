#include "LittleFS.h"
#define CONFIG_FILE "/interesting.cfg"
#define CONFIG_DIR "/config/"
#define WIFI_FILE "/wifi.cfg"

#define PROBABILITY_BASE 1000000

enum PixelType { Synced, Unsynced, Shifting, Adding };

// config parameters are written to file in this order
struct Config {
  PixelType PIXEL_TYPE = Adding;
  byte HUE_BITS = 5; // up to 16bit (65k hues), no point using more than 10bit (1024 hues)
  byte SATURATION = 255;
  byte VALUE = 63;
  // actually distinguishable hues per brightness level (=HSV value):
  // 63 -> 379
  // 127 -> 763
  // 255 -> 1531
  bool CORRECT_GAMMA = false;
  bool RANDOM_INTERVAL = true;
  
  uint16_t HUE_OFFSET = 0;
  uint16_t DHUE_MIN = 0;
  uint16_t DHUE_MAX = 0;
  // all durations in ms
  uint32_t P = 1000000;
  uint16_t ON_MIN = 2000;
  uint16_t ON_MAX = 4000;
  uint16_t OFF_TIME = 0;
  uint16_t FADE_IN = 0;
  uint16_t FADE_OUT = 0;
};

struct Config CONFIG;
String MSG = "everything is interesting once";

void loadConfig(String filename = CONFIG_FILE) {
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
  } else {
//    LittleFS.remove(CONFIG_FILE);
    if (filename.equals(CONFIG_FILE)) {
      if (File f = LittleFS.open(CONFIG_FILE, "r")) {
        Serial.println("Checking which config file to load: ");
        filename = f.readString();
        Serial.println(filename);
        f.close();
      } else {
        Serial.println("Didn't find a config file to tell us which config file to load");
        return;
      }
    }
    if (LittleFS.exists(filename)) {
      Serial.println("Loading config from " + filename);
      File f = LittleFS.open(filename, "r");
      f.readBytes((char *) &CONFIG, sizeof(struct Config));
/*      PIXEL_TYPE = (PixelType) f.read();
      HUE_BITS = f.read();
      SATURATION = f.read();
      VALUE = f.read();
      CORRECT_GAMMA = f.read();
      RANDOM_INTERVAL = f.read();
      HUE_OFFSET = f.parseInt();
      DHUE_MIN = f.parseInt();
      DHUE_MAX = f.parseInt();
      P = f.parseInt();
      ON_MIN = f.parseInt();
      ON_MAX = f.parseInt();
      OFF_TIME = f.parseInt();
      FADE_IN = f.parseInt();
      FADE_OUT = f.parseInt();*/
      MSG = f.readString();
      f.close();
    } else {
      Serial.println("FS mounted, but config file " + filename + " not found.");
    }
  }
}

void writeConfig(String filename) {
  Serial.println("Writing config file");
  if (File f = LittleFS.open(filename, "w")) {
    f.write((char *) &CONFIG, sizeof(struct Config));
  /*  f.write((byte) PIXEL_TYPE);
    f.write(HUE_BITS);
    f.write(SATURATION);
    f.write(VALUE);
    f.write((byte) CORRECT_GAMMA);
    f.write((byte) RANDOM_INTERVAL);
    f.println(HUE_OFFSET);
    f.println(DHUE_MIN);
    f.println(DHUE_MAX);
    f.println(P);
    f.println(ON_MIN);
    f.println(ON_MAX);
    f.println(OFF_TIME);
    f.println(FADE_IN);
    f.println(FADE_OUT);*/
    f.print(MSG);
    f.close();
    f = LittleFS.open(CONFIG_FILE, "w");
    f.print(filename);
    f.close();
  } else {
    Serial.println("Failed to write config file");
  }
}
