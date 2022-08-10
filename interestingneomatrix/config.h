#include "LittleFS.h"
#define CONFIG_FILE "/interesting.cfg"
#define CONFIG_DIR "/config/"
#define WIFI_FILE "/wifi.cfg"

#define N_PIXEL_TYPES 5
// TODO replace with switch and fadeoutin
String PIXEL_TYPES[N_PIXEL_TYPES] = { "syncedswitch", "fade to/from black", "rotate hue", "blend colors", "flicker" };
enum PixelType { Synced, FadeToBlack, Radial, Linear, Flicker };

// config parameters are written to file in this order
struct AnimationConfig {
  // all durations in ms
  uint16_t ON_TIME = 2000;
  uint16_t ON_EXTRA = 0;
  uint16_t OFF_TIME = 0;

  PixelType PIXEL_TYPE = Linear;
  // switching probability -- ms
  uint16_t LIMIT_CHANGES = 1;

  uint16_t TRANSITION_DURATION = 1;
  uint16_t TRANSITION_EXTRA = 0;
  // if true, only hue transitions of 180deg actually take the full transition duration
  bool PACE_TRANSITIONS = false;
  bool SINE_TRANSITIONS = false;

  // noise that goes in both directions
  uint8_t HUE_DRIFT = 0;
  // this one goes in both directions
  byte DHUE_MIN = 15;
};

struct Config {
//  PixelType PIXEL_TYPE = Linear;
  byte HUE_BITS = 8; // up to 16bit (65k hues), no point using more than 10bit (1024 hues)
  byte SATURATION = 255;
  byte VALUE = 127;
  // actually distinguishable hues per brightness level (=HSV value):
  // 63 -> 379
  // 127 -> 763
  // 255 -> 1531
  bool CORRECT_GAMMA = false;
  bool SYNC_PIXELS = false;

//  uint16_t TRANSITION_DURATION = 0;
//  bool PACE_TRANSITIONS = true;

  byte HUE_OFFSET = 0;
  byte DHUE_DETERMINISTIC = 0;

  struct AnimationConfig ANIMATION[2];
};

struct Config CONFIG;
String CONFIG_FILE_NAME;
String MSG = "everything is interesting once";

#define PROBABILITY_BASE 1000000
uint32_t transitionPs[2] = { PROBABILITY_BASE, PROBABILITY_BASE };

// also sets CONFIG_FILE_NAME to something valid
void loadConfig(String filename = CONFIG_FILE) {
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
  } else {
    if (filename.equals(CONFIG_FILE) || filename.equals("")) {
      if (File f = LittleFS.open(CONFIG_FILE, "r")) {
//        LittleFS.remove(CONFIG_FILE);
        Serial.print("Checking which config file to load: ");
        filename = f.readString();
        Serial.println(filename);
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
      CONFIG_FILE_NAME = filename;
    } else {
      Serial.println("FS mounted, but config file " + filename + " not found.");
    }
  }
}

void setDefaultConfig(String filename = CONFIG_FILE_NAME) {
  File f = LittleFS.open(CONFIG_FILE, "w");
  f.print(filename);
  f.close();
}

void deleteConfig(String filename) {
  LittleFS.remove(CONFIG_DIR + filename);
}

void writeConfig(String filename) {
  Serial.println("Writing config file: " + filename);
  if (File f = LittleFS.open(CONFIG_DIR + filename, "w")) {
    f.write((char *) &CONFIG, sizeof(struct Config));
    f.print(MSG);
    f.close();
    CONFIG_FILE_NAME = filename;
    setDefaultConfig();
  } else {
    Serial.println("Failed to write config file");
  }
}
