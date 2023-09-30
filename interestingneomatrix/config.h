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
  uint16_t ON_TIME = 1000;
  uint16_t ON_EXTRA = 0;
  uint16_t OFF_TIME = 800;

  PixelType PIXEL_TYPE = Linear;
  // switching probability -- ms
  uint16_t LIMIT_CHANGES = 1200;

  uint16_t TRANSITION_DURATION = 1000;
  uint16_t TRANSITION_EXTRA = 0;
  // if true, only hue transitions of 180deg actually take the full transition duration
  bool PACE_TRANSITIONS = false;
  bool SINE_TRANSITIONS = false;

  // noise that goes in both directions
  uint8_t HUE_DRIFT = 20;
  // this one goes in both directions
  uint8_t DHUE_MIN = 60;
};

struct Config {
  uint8_t HUE_BITS = 6; // up to 16bit (65k hues), no point using more than 10bit (1024 hues)
  uint8_t SATURATION = 255;
  uint8_t VALUE = 64;
  // actually distinguishable hues per brightness level (=HSV value):
  // 63 -> 379
  // 127 -> 763
  // 255 -> 1531
  bool CORRECT_GAMMA = false;
  bool SYNC_PIXELS = false;

  uint8_t HUE_OFFSET = 0;
  uint8_t DHUE_DETERMINISTIC = 0;

  struct AnimationConfig ANIMATION[2];
};

struct Config CONFIG;

String CONFIG_FILE_NAME;
#define DEFAULT_MSG "everything is interesting once"
String MSG = DEFAULT_MSG;

#define PROBABILITY_BASE 1000000
uint32_t transitionPs[2] = { PROBABILITY_BASE, PROBABILITY_BASE };

// also sets CONFIG_FILE_NAME to something valid
void loadConfig(String filename = CONFIG_FILE) {
  // set pretty defaults
  CONFIG.ANIMATION[0].PIXEL_TYPE = FadeToBlack;
  CONFIG.ANIMATION[0].TRANSITION_DURATION = 2000;
  CONFIG.ANIMATION[0].TRANSITION_EXTRA = 100;
  CONFIG.ANIMATION[1].ON_TIME = 300;
  CONFIG.ANIMATION[1].TRANSITION_EXTRA = 500;
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
