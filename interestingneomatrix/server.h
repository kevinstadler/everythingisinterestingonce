#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

String fetchCallback(String target, String field = "value", String extraCallback = "") {
  return "\"fetch('" + target + "', { method: 'POST', body: this." + field + " } ).then(r => {" + extraCallback + "})\"";
}

String slider(String id, String desc, byte mn, uint16_t mx, uint16_t value, String unit = "", byte step = 1, bool enabled = true) {
  // orient="vertical"
  String attribs = "value=\"" + String(value) + "\" min=\"" + String(mn) + "\" max=\"" + String(mx) + "\" step=\"" + String(step) + "\"";
  return desc + " <input type=\"range\" id=\"" + id + "\" " + attribs + " oninput=" + fetchCallback(id, "value", "this.nextElementSibling.value = this.value;") +
    (enabled ? "" : " disabled") +"> <input type=\"number\" id=\"" + id + "_\" " + attribs + " \" oninput=" + fetchCallback(id, "value", "this.previousElementSibling.value = this.value;") + ">" + unit + "<br>";
}

String button(String id, String label, bool onclick = true) {
  return "<input type=\"" + String(id.equals("submit") ? "submit" : "button") + "\" id=\"" + id + "\" value=\"" + label + "\" onclick=" + (onclick ? fetchCallback(id) : "") + ">";
}

String checkbox(String id, bool checked, String desc, String extraCallback = "") {
  return "<input type=\"checkbox\" id=\"" + id + "\" onclick=" + fetchCallback(id, "checked", extraCallback) +
    (checked ? " checked" : "") + "> <label for=\"" + id + "\">" + desc + "</label><br>";
}

String textarea(String filename, String content) {
  return "<textarea rows=\"8\" cols=\"40\" id=\"content\" name=\"content\">" + content + "</textarea><input type=\"hidden\" name=\"filename\" value=\"" + filename + "\">";
}

#define HEADER "<html>\
  <head>\
    <title>everything has a web interface once</title>\
    <style>\
      html {  width: fit-content; margin: auto; }\
      body, input { background-color: black; color: #ccc; font-family: Courier, serif; text-align: center; margin: 1em .5em; }\
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
  String form = "<input type=\"text\" style=\"font-size: 200%; width: 30em; text-align: center;\" value=\""
    + MSG + "\" oninput=" + fetchCallback("msg") + "><br>";
  form += slider("0", "hue offset (deg)", 0, 360, CONFIG.HUE_OFFSET/182, "/360");
  form += slider("h", "hue resolution (bits)", 0, 16, CONFIG.HUE_BITS, "/16");
  form += slider("s", "saturation", 0, 255, CONFIG.SATURATION, "/255");
  form += slider("v", "value/brightness", 1, 255, CONFIG.VALUE, "/255");
//  form += checkbox("g", CORRECT_GAMMA, "gamma-correct hues");
  form += slider("fin", "fade-in time", 0, 1000, CONFIG.FADE_IN, " ms", 10);
  form += slider("on1", "on time", 10, 4000, CONFIG.ON_MIN, " ms", 50);
  form += checkbox("r", CONFIG.RANDOM_INTERVAL, "randomize intervals", "document.getElementById('on2').disabled = (this.value === 'false')");
  form += slider("on2", "on time (upper bound)", 10, 4000, CONFIG.ON_MAX, " ms", 50, CONFIG.RANDOM_INTERVAL);
  form += slider("fout", "fade-out time", 0, 1000, CONFIG.FADE_OUT, " ms", 10);
  form += slider("off", "off time", 0, 4000, CONFIG.OFF_TIME, " ms", 10);
  Dir dir = LittleFS.openDir(CONFIG_DIR);
  while (dir.next()) {
    Serial.println(dir.fileName());
  }
  String options[5];
//  form += dropdown("load", options);
  form += button("p", "save current config");
  form += button("x", "load last config");
  form += button("z", "restart");
  sendForm(form);
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
      case 'm':
        setMsg(val); break;
      case '0':
        // from [0,360) to [0, 65536)
        CONFIG.HUE_OFFSET = 182*x; break;
      case 'h':
        CONFIG.HUE_BITS = x; break;
      case 's':
        CONFIG.SATURATION = x; break;
      case 'v':
        CONFIG.VALUE = x; break;
      case 'g':
        CONFIG.CORRECT_GAMMA = server.arg(0).equals("true"); break;
      case 'r':
        CONFIG.RANDOM_INTERVAL = server.arg(0).equals("true"); break;
      case 'o':
        if (target.equals("on1")) {
          CONFIG.ON_MIN = x;
        } else if (target.equals("on2")) {
          CONFIG.ON_MAX = x;
        } else if (target.equals("off")) {
          CONFIG.OFF_TIME = x;
        }
        break;
      case 'f':
        if (target.equals("fin")) {
          CONFIG.FADE_IN = x;
        } else if (target.equals("fout")) {
          CONFIG.FADE_OUT = x;
        }
        break;
      case 'p':
        writeConfig(val); break;
      case 'w':
        if (File f = LittleFS.open(server.arg("filename"), "w")) {
          f.println(server.arg("content"));
          f.close();
        }
        break;
      case 'x':
        loadConfig(); break;
      case 'z':
        Serial.println("Restarting");
        ESP.reset();
    }
    server.send(200, "text/plain", val);
  }
}

void startInterface() {
  server.on("/", sendConfigForm);
  server.onNotFound(handleForm);

  server.begin();
}
