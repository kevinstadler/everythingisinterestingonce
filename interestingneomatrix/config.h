#include "LittleFS.h"
#define CONFIG_FILE "/interesting.cfg"
#define CONFIG_DIR "/config/"
#define WIFI_FILE "/wifi.cfg"

String PIXEL_TYPES[4] = {"synced", "unsynced", "radial blend", "linear blend"};
enum PixelType { Synced, Unsynced, Radial, Linear };

// config parameters are written to file in this order
struct Config {
  PixelType PIXEL_TYPE = Linear;
  byte HUE_BITS = 6; // up to 16bit (65k hues), no point using more than 10bit (1024 hues)
  byte SATURATION = 255;
  byte VALUE = 127;
  // actually distinguishable hues per brightness level (=HSV value):
  // 63 -> 379
  // 127 -> 763
  // 255 -> 1531
  bool CORRECT_GAMMA = false;
//  bool SYNC_PIXELS = false;

  // all durations in ms
  uint16_t ON_MIN = 2000;
  uint16_t ON_MAX = 4000;
//  uint16_t OFF_TIME = 0;
  bool RANDOM_INTERVAL = true;

  // switching probability
  uint16_t LIMIT_CHANGES = 1;

  uint16_t TRANSITION_DURATION = 0;
  bool PACE_TRANSITIONS = true;

  byte HUE_OFFSET = 0;
  byte DHUE_DETERMINISTIC = 0;
  // this one goes in both directions
  byte DHUE_MIN = 0;
  byte HUE_DRIFT = 0;

};

struct Config CONFIG;
String CONFIG_FILE_NAME;
String MSG = "everything is interesting once";

#define PROBABILITY_BASE 1000000
uint32_t transitionP = PROBABILITY_BASE;

void loadConfig(String filename = CONFIG_FILE) {
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
  } else {
//    LittleFS.remove(CONFIG_FILE);
    if (filename.equals(CONFIG_FILE) || filename.equals("")) {
      if (File f = LittleFS.open(CONFIG_FILE, "r")) {
        Serial.print("Checking which config file to load: ");
        filename = f.readString();
        Serial.println(filename);
        CONFIG_FILE_NAME = filename;
        f.close();
      } else {
        Serial.println("Didn't find a config file to tell us which config file to load");
        return;
      }
    }
    if (LittleFS.exists(CONFIG_DIR + filename)) {
      Serial.println("Loading config from " + filename);
      File f = LittleFS.open(CONFIG_DIR + filename, "r");
      f.readBytes((char *) &CONFIG, sizeof(struct Config));
      MSG = f.readString();
      f.close();
    } else {
      Serial.println("FS mounted, but config file " + filename + " not found.");
    }
  }
}

void writeConfig(String filename) {
  Serial.println("Writing config file");
  if (File f = LittleFS.open(CONFIG_DIR + filename, "w")) {
    f.write((char *) &CONFIG, sizeof(struct Config));
    f.print(MSG);
    f.close();
    CONFIG_FILE_NAME = filename;
    f = LittleFS.open(CONFIG_FILE, "w");
    f.print(filename);
    f.close();
  } else {
    Serial.println("Failed to write config file");
  }
}
