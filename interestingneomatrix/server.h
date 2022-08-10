#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

String fetchCallback(String target, String field = "value", String extraCallback = "") {
  return "\"fetch('" + target + "', { method: 'POST', body: this." + field + " } ).then(r => {" + extraCallback + "})\"";
}

String input(String id, String value, String type = "text", String style = "", String callback = "defaultFetchCallback") {
  return "<input type=\"" + type + "\" value=\"" + value + "\" oninput=" + (callback.equals("defaultFetchCallback") ? fetchCallback(id) : "\"" + callback + "\"") + " style=\"" + style + "\">";
}

String slider(String id, String desc, byte mn, uint16_t mx, uint16_t value, String unit = "", byte step = 1, bool enabled = true) {
  // orient="vertical"
  String attribs = "value=\"" + String(value) + "\" min=\"" + String(mn) + "\" max=\"" + String(mx) + "\" step=\"" + String(step) + "\"";
  return desc + " <input type=\"range\" id=\"" + id + "\" " + attribs + " oninput=" + fetchCallback(id, "value", "this.nextElementSibling.value = this.value;") +
    (enabled ? "" : " disabled") +"> <input type=\"number\" id=\"" + id + "_\" " + attribs + " \" oninput=" + fetchCallback(id, "value", "this.previousElementSibling.value = this.value;") + ">" + unit + "<br>";
}

String button(String id, String label, bool fetchNotSubmit = true, String callback = "") {
  if (fetchNotSubmit && callback.equals("")) {
    callback = fetchCallback(id);
  }
  return "<input type=\"" + String(id.equals("submit") ? "submit" : "button") + "\" id=\"" + id + "\" value=\"" + label + "\" onclick=" + (fetchNotSubmit ? callback : "") + ">";
}

String checkbox(String id, bool checked, String desc, String extraCallback = "") {
  return "<input type=\"checkbox\" id=\"" + id + "\" onclick=" + fetchCallback(id, "checked", extraCallback) +
    (checked ? " checked" : "") + "> <label for=\"" + id + "\">" + desc + "</label><br>";
}

String toggle(String id) {
  // https://www.w3schools.com/howto/howto_css_switch.asp
  return "";
}

String dropdown(String id, String options[], byte n, byte selectedIndex, byte startAt = 0) {
  String s = "<select onchange=" + fetchCallback(id, "value") + ">";
  for (byte i = startAt; i < n; i++) {
    s += "<option value=\"" + String(i) + "\"" + ((i == selectedIndex) ? " selected" : "") + ">" + options[i] + "</option>";
  }
  return s + "</select>";
}

String textarea(String filename, String content) {
  return "<textarea rows=\"8\" cols=\"40\" id=\"content\" name=\"content\">" + content + "</textarea><input type=\"hidden\" name=\"filename\" value=\"" + filename + "\">";
}

#define HEADER "<html>\
  <head>\
    <title>everything has a web interface once</title>\
    <style>\
      html {  width: fit-content; margin: auto; }\
      body, input, select, td { background-color: black; color: #ccc; font-family: Courier, serif; text-align: center; margin: 1em .5em; }\
    </style>\
  </head>\
  <body>"

#define FOOTER "  </body></html>"

void sendForm(String content) {
  // https://github.com/esp8266/Arduino/issues/3205
  server.send(200, "text/html", HEADER + content + FOOTER);
}

String form(String action, String content) {
  return "<form action=\"" + action + "\" method=\"post\">" + content + "</form>";
}

String animationConfig(uint8_t i) {
  return "transition type: " + dropdown(String(i) + "t", PIXEL_TYPES, N_PIXEL_TYPES, (byte) CONFIG.ANIMATION[i].PIXEL_TYPE, 1) + "<br>"
    + slider(String(i) + "l", "don't transition more than every ", 1, 5000, CONFIG.ANIMATION[i].LIMIT_CHANGES, "ms")
    + slider(String(i) + "q", "minimum transition duration", 1, 10000, CONFIG.ANIMATION[i].TRANSITION_DURATION, "ms", 50)
    + slider(String(i) + "r", "random extra time", 0, 10000, CONFIG.ANIMATION[i].TRANSITION_EXTRA, "ms", 50)
    + checkbox(String(i) + "y", CONFIG.ANIMATION[i].PACE_TRANSITIONS, "pace based on color distance")
    + checkbox(String(i) + "s", CONFIG.ANIMATION[i].SINE_TRANSITIONS, "use sinusoid transitions")
    + slider(String(i) + "off", "off time / non-paced transition time", 0, 4000, CONFIG.ANIMATION[i].OFF_TIME, " ms", 50)
    + slider(String(i) + "d", "drift / noise", 0, 255, CONFIG.ANIMATION[i].HUE_DRIFT, "/255")
    + slider(String(i) + "n", "minimum hue change", 0, 127, CONFIG.ANIMATION[i].DHUE_MIN, "/127")
    + slider(String(i) + "on1", "fixed on time", 10, 4000, CONFIG.ANIMATION[i].ON_TIME, " ms", 50)
    + slider(String(i) + "on2", "random extra time", 0, 4000, CONFIG.ANIMATION[i].ON_EXTRA, " ms", 50);
}

void sendConfigForm() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", HEADER);

  server.sendContent(input("msg", MSG, "text", "font-size: 200%; width: 30em; text-align: center;") + "<br>"
//    + dropdown("t", PIXEL_TYPES, N_PIXEL_TYPES, (byte) CONFIG.PIXEL_TYPE) + "<br>"
//    + slider("o", "hue offset (deg)", 0, 360, CONFIG.HUE_OFFSET/182, "/360")
    + slider("h", "hue resolution (bits)", 0, 16, CONFIG.HUE_BITS, "/16")
    + slider("s", "saturation", 0, 255, CONFIG.SATURATION, "/255")
    + slider("v", "value/brightness", 1, 255, CONFIG.VALUE, "/255")
//    + checkbox("g", CONFIG.CORRECT_GAMMA, "apply gamma correction")
  //  + slider("fin", "fade-in time", 0, 1000, CONFIG.FADE_IN, " ms", 10)
  //  + slider("fout", "fade-out time", 0, 1000, CONFIG.FADE_OUT, " ms", 10)
//  +  dropdown("load", options);
//    + checkbox("TODO", CONFIG.SYNC_PIXELS, "sync all pixels")
    + "<table><tr><td style=\"padding-right: 4em;\">" + animationConfig(0) + "</td><td>" + animationConfig(1) + "</td></tr></table>"
    + input("filename", CONFIG_FILE_NAME) + button("config", "save current config", true, fetchCallback("config", "previousElementSibling.value")) + "<br>"
    + "config settings: <select id=\"config\"><option></option>");
  Dir dir = LittleFS.openDir(CONFIG_DIR);
  while (dir.next()) {
    server.sendContent("<option value=\"" + dir.fileName() + "\"" + (dir.fileName().equals(CONFIG_FILE_NAME) ? " selected" : "") + ">" + dir.fileName() + "</option>");
  }
  server.sendContent("</select>"
    + button("load", "load", true, fetchCallback("load", "parentNode.querySelector('#config').value", "location.reload()"))
    + button("default", "set as default", true, fetchCallback("default", "parentNode.querySelector('#config').value", "location.reload()"))
    + button("erase", "delete", true, fetchCallback("erase", "parentNode.querySelector('#config').value", "location.reload()"))
    + "<br>" + button("r", "restart"));
  server.sendContent(FOOTER);
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    Serial.println(server.uri());
    if (server.uri().equals("/wifi")) {
      String content = "";
      if (File f = LittleFS.open(WIFI_FILE, "r")) {
        content = f.readString();
        f.close();
      }
      sendForm(form("wifi", textarea(WIFI_FILE, content) + button("submit", "update networks", false)));
    } else {
      server.send(405, "text/plain", "Method Not Allowed");
    }
  } else {
    String val = server.arg("plain");
    uint16_t x = val.toInt();
    Serial.println(server.uri() + ": " + val);

    if (server.uri().charAt(1) == '0' || server.uri().charAt(1) == '1') {
      // animation config
      uint8_t animation = server.uri().substring(1, 2).toInt();
      String target = server.uri().substring(2);

      switch (target.charAt(0)) {
        case 'd':
          CONFIG.ANIMATION[animation].HUE_DRIFT = x;
          // TODO set as default config
          break;
        case 'l':
          CONFIG.ANIMATION[animation].LIMIT_CHANGES = x;
          setMsg();
          break;
        case 'n':
          CONFIG.ANIMATION[animation].DHUE_MIN = x; break;
        case 'o':
          if (target.equals("on1")) {
            CONFIG.ANIMATION[animation].ON_TIME = x;
          } else if (target.equals("on2")) {
            CONFIG.ANIMATION[animation].ON_EXTRA = x;
          } else if (target.equals("off")) {
            CONFIG.ANIMATION[animation].OFF_TIME = x;
          }
          break;
        case 'q':
          CONFIG.ANIMATION[animation].TRANSITION_DURATION = x; break;
        case 'r':
          CONFIG.ANIMATION[animation].TRANSITION_EXTRA = x; break;
        case 's':
          CONFIG.ANIMATION[animation].SINE_TRANSITIONS = server.arg(0).equals("true"); break;
        case 't':
          CONFIG.ANIMATION[animation].PIXEL_TYPE = (PixelType) x;
          setMsg();
          break;
        case 'y':
          CONFIG.ANIMATION[animation].PACE_TRANSITIONS = server.arg(0).equals("true"); break;
        default:
          Serial.println("unknown animation POST request!");
      }
      
    } else {
      // global config
      String target = server.uri().substring(1);
      switch (target.charAt(0)) {
        case 'c':
          writeConfig(val); break;
        case 'd':
          setDefaultConfig(val); break;
        case 'e':
          deleteConfig(val); break;
        case 'g':
          CONFIG.CORRECT_GAMMA = server.arg(0).equals("true"); break;
        case 'h':
          CONFIG.HUE_BITS = x; break;
        case 'l':
          loadConfig(val); break;
        case 'm':
          setMsg(val); break;
        case 'o':
          #ifdef FASTLED
          // from [0,360) to [0, 255) -- ROUGH
          CONFIG.HUE_OFFSET = (x*7)/10; break;
          #else
          // from [0,360) to [0, 65536)
          CONFIG.HUE_OFFSET = 182*x; break;
          #endif
        case 'r':
          Serial.println("Restarting");
          ESP.reset();
        case 's':
          CONFIG.SATURATION = x; break;
        case 'v':
          CONFIG.VALUE = x; break;
        case 'w':
          if (File f = LittleFS.open(server.arg("filename"), "w")) {
            f.println(server.arg("content"));
            f.close();
          }
          break;
        default:
          Serial.println("unknown global POST request!");
      }
    }
/*      case 'f':
        if (target.equals("fin")) {
          CONFIG.FADE_IN = x;
        } else if (target.equals("fout")) {
          CONFIG.FADE_OUT = x;
        }
        break;
        */
    server.send(200, "text/plain", val);
  }
}

void startInterface() {
  server.on("/", sendConfigForm);
  server.onNotFound(handleForm);

  server.begin();
}
