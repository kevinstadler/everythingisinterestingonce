//#include <FS.h>
#include <ESPAsyncTCP.h>

#include <ESPAsyncWebSrv.h>

AsyncWebServer server(80);

String processor(const String& var) {
  uint16_t i = 0;
  if (var == "msg"){
    return MSG;
  } else if (var == "resolution") {
    i = CONFIG.HUE_BITS;
  } else if (var == "crossfade") {
    i = CONFIG.ANIMATION[0].LIMIT_CHANGES;
  } else if (var == "radial") {
    return CONFIG.ANIMATION[0].PIXEL_TYPE == Radial ? "checked" : ""; // or default: Linear aka Peachy
  } else if (var == "flicker") {
    return CONFIG.ANIMATION[1].PIXEL_TYPE == Flicker ? "checked" : ""; // or FadeToBlack
  } else if (var == "0on1") {
    i = CONFIG.ANIMATION[0].ON_TIME;
  } else if (var == "0on2") {
    i = CONFIG.ANIMATION[1].ON_EXTRA;
  } else if (var == "1on1") {
    i = CONFIG.ANIMATION[1].ON_TIME;
  } else if (var == "1on2") {
    i = CONFIG.ANIMATION[1].ON_EXTRA;
  } else if (var == "0q") {
    i = CONFIG.ANIMATION[0].TRANSITION_DURATION;
  } else if (var == "0r") {
    i = CONFIG.ANIMATION[0].TRANSITION_EXTRA;
  } else if (var == "0off") {
    i = CONFIG.ANIMATION[0].OFF_TIME;
  } else if (var == "1q") {
    i = CONFIG.ANIMATION[1].TRANSITION_DURATION;
  } else if (var == "1r") {
    i = CONFIG.ANIMATION[1].TRANSITION_EXTRA;
  } else if (var == "1off") {
    i = CONFIG.ANIMATION[1].OFF_TIME;
  } else if (var == "0n") {
    i = CONFIG.ANIMATION[0].DHUE_MIN;
  } else if (var == "0d") {
    i = CONFIG.ANIMATION[0].HUE_DRIFT;
  } else if (var == "1n") {
    i = CONFIG.ANIMATION[1].DHUE_MIN;
  } else if (var == "1d") {
    i = CONFIG.ANIMATION[1].HUE_DRIFT;
  }
  return String(i);
}

void startInterface() {
  server.serveStatic("/fs", LittleFS, "/");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    // null-terminate just in case
    char val[len+1];
    val[len] = '\0';
    memcpy(val, data, len * sizeof(uint8_t));
    uint16_t x = atoi(val);
    if (request->url()[1] == '0' || request->url()[1] == '1') {
      // animation config
      uint8_t animation = request->url()[1] - 48; // ascii hack
      switch (request->url()[2]) {
        case 'd':
          // TODO set as default config
          CONFIG.ANIMATION[animation].HUE_DRIFT = x; break;
        case 'l':
          CONFIG.ANIMATION[animation].LIMIT_CHANGES = x;
          calculateTransitionPs();
          break;
        case 'n':
          CONFIG.ANIMATION[animation].DHUE_MIN = x; break;
        case 'o': // on1, on2 or off
          switch (request->url()[4]) {
            case '1':
              CONFIG.ANIMATION[animation].ON_TIME = x; break;
            case '2':
              CONFIG.ANIMATION[animation].ON_EXTRA = x; break;
            case 'f':
              CONFIG.ANIMATION[animation].OFF_TIME = x; break;
          }
          break;
        case 'q':
          CONFIG.ANIMATION[animation].TRANSITION_DURATION = x; break;
        case 'r':
          CONFIG.ANIMATION[animation].TRANSITION_EXTRA = x; break;
//        case 's':
//          CONFIG.ANIMATION[animation].SINE_TRANSITIONS = server.arg(0).equals("true"); break;
        case 't':
          CONFIG.ANIMATION[animation].PIXEL_TYPE = (PixelType) x;
          break;
//        case 'y':
//          CONFIG.ANIMATION[animation].PACE_TRANSITIONS = server.arg(0).equals("true"); break;
        default:
          LOG.println("unknown animation POST request!");
      }
    } else {
      // global config
      switch (request->url()[1]) {
        case 'c':
          writeConfig(val); break;
        case 'd':
          setDefaultConfig(val); break;
        case 'e':
          deleteConfig(val); break;
//        case 'g':
//          CONFIG.CORRECT_GAMMA = server.arg(0).equals("true"); break;
        case 'h':
          CONFIG.HUE_BITS = x; break;
        case 'l':
          loadConfig(val); break;
        case 'm':
          setMsg(String(val)); break;
        case 'o':
          CONFIG.HUE_OFFSET = (x*7)/10; break;
        case 'r':
          ESP.reset();
        case 's':
          CONFIG.SATURATION = x; break;
        case 'v':
          CONFIG.VALUE = x; break;
        case 'w':
//          if (File f = LittleFS.open(server.arg("filename"), "w")) {
//            f.println(server.arg("content"));
//            f.close();
//          }
          break;
        default:
          LOG.println("unknown global POST request!");
      }
    }
    request->send(200, "text/plain", val);

  });

  server.begin();
}
