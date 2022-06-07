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

String dropdown(String id, String options[], byte n, byte selectedIndex) {
  String s = "<select onchange=" + fetchCallback(id, "value") + ">";
  for (byte i = 0; i < n; i++) {
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
      body, input, select { background-color: black; color: #ccc; font-family: Courier, serif; text-align: center; margin: 1em .5em; }\
    </style>\
  </head>\
  <body>"

#define FOOTER "  </body></html>"

void sendForm(String content) {
  server.send(200, "text/html", HEADER + content + FOOTER);
}

String form(String action, String content) {
  return "<form action=\"" + action + "\" method=\"post\">" + content + "</form>";
}

void sendConfigForm() {
  String configForm = input("msg", MSG, "text", "font-size: 200%; width: 30em; text-align: center;") + "<br>pixel animation style: "
    + dropdown("t", PIXEL_TYPES, 4, (byte) CONFIG.PIXEL_TYPE) + "<br>"
    + slider("0", "hue offset (deg)", 0, 360, CONFIG.HUE_OFFSET/182, "/360")
    + slider("h", "hue resolution (bits)", 0, 16, CONFIG.HUE_BITS, "/16")
    + slider("n", "minimum hue change", 0, 128, CONFIG.DHUE_MIN, "/128")
    + slider("s", "saturation", 0, 255, CONFIG.SATURATION, "/255")
    + slider("v", "value/brightness", 1, 255, CONFIG.VALUE, "/255")
  //  + checkbox("g", CORRECT_GAMMA, "gamma-correct hues");
  //  + slider("fin", "fade-in time", 0, 1000, CONFIG.FADE_IN, " ms", 10)
    + slider("on1", "on time", 10, 4000, CONFIG.ON_MIN, " ms", 50)
    + checkbox("r", CONFIG.RANDOM_INTERVAL, "randomize intervals", "document.getElementById('on2').disabled = (this.value === 'false')")
    + slider("on2", "on time (upper bound)", 10, 4000, CONFIG.ON_MAX, " ms", 50, CONFIG.RANDOM_INTERVAL)
    + slider("p", "globally limit changes to one every ", 1, 10000, CONFIG.LIMIT_CHANGES, "ms")
    + slider("q", "transition duration", 0, 10000, CONFIG.TRANSITION_DURATION, "ms")
    + checkbox("y", CONFIG.PACE_TRANSITIONS, "pace transitions");
  //  + slider("fout", "fade-out time", 0, 1000, CONFIG.FADE_OUT, " ms", 10)
  //  + slider("off", "off time", 0, 4000, CONFIG.OFF_TIME, " ms", 10);
//  +  dropdown("load", options);
  configForm += input("filename", CONFIG_FILE_NAME) + button("config", "save current config", true, fetchCallback("config", "previousElementSibling.value")) + "<br>"
    + "<select onchange=\"" + + "\"><option></option>";
  Dir dir = LittleFS.openDir(CONFIG_DIR);
  while (dir.next()) {
    configForm += "<option value=\"" + dir.fileName() + "\"" + (dir.fileName().equals(CONFIG_FILE_NAME) ? " selected" : "") + ">" + dir.fileName() + "</option>";
  }
  configForm += "</select>" + button("load", "load config", true, fetchCallback("load", "previousElementSibling.value", "location.reload()"))
  // TODO add button("p", "set as default")
    + "<br>" + button("z", "restart");
  sendForm(configForm);
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
    Serial.println(val);

    String target = server.uri().substring(1);
    switch (target.charAt(0)) {
      case '0':
        #ifdef FASTLED
        // from [0,360) to [0, 255) -- ROUGH
        CONFIG.HUE_OFFSET = (x*7)/10; break;
        #else
        // from [0,360) to [0, 65536)
        CONFIG.HUE_OFFSET = 182*x; break;
        #endif
      case 'c':
        writeConfig(val); break;
      case 'd':
        // TODO set as default config
        break;
      case 'g':
        CONFIG.CORRECT_GAMMA = server.arg(0).equals("true"); break;
      case 'h':
        CONFIG.HUE_BITS = x; break;
      case 'l':
        loadConfig(val); break;
      case 'm':
        setMsg(val); break;
      case 'n':
        CONFIG.DHUE_MIN = x; break;
      case 'o':
        if (target.equals("on1")) {
          CONFIG.ON_MIN = x;
        } else if (target.equals("on2")) {
          CONFIG.ON_MAX = x;
        } else if (target.equals("off")) {
//          CONFIG.OFF_TIME = x;
        }
        break;
/*      case 'f':
        if (target.equals("fin")) {
          CONFIG.FADE_IN = x;
        } else if (target.equals("fout")) {
          CONFIG.FADE_OUT = x;
        }
        break;
        */
      case 'p':
        CONFIG.LIMIT_CHANGES = x;
        setMsg();
        break;
      case 'q':
        CONFIG.TRANSITION_DURATION = x; break;
      case 'r':
        CONFIG.RANDOM_INTERVAL = server.arg(0).equals("true"); break;
      case 's':
        CONFIG.SATURATION = x; break;
      case 't':
        CONFIG.PIXEL_TYPE = (PixelType) x;
        setMsg();
        break;
      case 'v':
        CONFIG.VALUE = x; break;
      case 'w':
        if (File f = LittleFS.open(server.arg("filename"), "w")) {
          f.println(server.arg("content"));
          f.close();
        }
        break;
      case 'y':
        CONFIG.PACE_TRANSITIONS = server.arg(0).equals("true"); break;
      case 'z':
        Serial.println("Restarting");
        ESP.reset();
      default:
        Serial.println("unknown POST request!");
    }
    server.send(200, "text/plain", val);
  }
}

void startInterface() {
  server.on("/", sendConfigForm);
  server.onNotFound(handleForm);

  server.begin();
}
