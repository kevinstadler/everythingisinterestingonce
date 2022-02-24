#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(80);

String fetchCallback(String target, String field = "value", String extraCallback = "") {
  return "\"fetch('" + target + "', { method: 'POST', body: this." + field + " } ).then(r => {" + extraCallback + "})\"";
}

String slider(String id, String desc, byte mn, uint16_t mx, uint16_t value, String unit = "", byte step = 1, bool enabled = true) {
  return desc + " <input type=\"range\" id=\"" + id + "\" min=\"" + String(mn) + "\" max=\"" + String(mx) + "\" step=\"" + String(step) + "\" value=\"" + String(value) + "\" oninput=" + fetchCallback(id, "value", "this.nextElementSibling.innerHTML = this.value;") + (enabled ? "" : " disabled") +"> <span>" + String(value) + "</span>" + unit + "<br>";
}

String button(String id, String label) {
  return "<input type=\"button\" id=\"" + id + "\" value=\"" + label + "\" onclick=" + fetchCallback(id) + ">";
}

String checkbox(String id, bool checked, String desc, String extraCallback = "") {
  return "<input type=\"checkbox\" id=\"" + id + "\" onclick=" + fetchCallback(id, "checked", extraCallback) + (checked ? " checked" : "") + "> <label for=\"" + id + "\">" + desc + "</label><br>";
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

void configForm() {
  String form = HEADER;
  form += "<input type=\"text\" style=\"font-size: 200%; width: 30em; text-align: center;\" value=\"" + MSG + "\" oninput=" + fetchCallback("msg") + "><br>";
  form += slider("h", "hue bits", 1, 16, HUE_BITS, "/16");
  form += slider("s", "saturation", 0, 255, SATURATION, "/255");
  form += slider("v", "value (brightness)", 1, 255, VALUE, "/255");
  form += checkbox("g", CORRECT_GAMMA, "gamma-correct hues");
  form += slider("on1", "on time", 10, 4000, ON_MIN, " ms", 50);
  form += checkbox("r", RANDOM_INTERVAL, "randomize intervals", "document.getElementById('on2').disabled = (this.value === 'false')");
  form += slider("on2", "on time (upper bound)", 10, 4000, ON_MAX, " ms", 50, RANDOM_INTERVAL);
  form += slider("off", "off time", 0, 4000, OFF_TIME, " ms", 10);
  form += button("p", "save config");
  form += button("x", "load last config");
  form += FOOTER;
  server.send(200, "text/html", form);
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String val = server.arg("plain");
    uint16_t x = val.toInt();
    Serial.println(val);

    String target = server.uri().substring(1);
    switch (target.charAt(0)) {
      case 'm':
        setMsg(val); break;
      case 'h':
        HUE_BITS = x; break;
      case 's':
        SATURATION = x; break;
      case 'v':
        VALUE = x; break;
      case 'g':
        CORRECT_GAMMA = server.arg(0).equals("true"); break;
      case 'r':
        RANDOM_INTERVAL = server.arg(0).equals("true"); break;
      case 'o':
        if (target.equals("on1")) {
          ON_MIN = x;
        } else if (target.equals("on2")) {
          ON_MAX = x;
        } else if (target.equals("off")) {
          OFF_TIME = x;
        }
        break;
      case 'p':
        persistConfig(); break;
      case 'x':
        loadConfig(); break;
    }
    draw();
    server.send(200, "text/plain", val);
  }
}

void startInterface() {
// IPAddress local_IP(10, 0, 0, 1);
//  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.println(WiFi.softAP("everythingisinterestingonce") ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  if (MDNS.begin("home")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", configForm);

  server.on("/msg", handleForm);
  server.on("/h", handleForm);
  server.on("/s", handleForm);
  server.on("/v", handleForm);
  server.on("/g", handleForm);
  server.on("/on1", handleForm);
  server.on("/r", handleForm);
  server.on("/on2", handleForm);
  server.on("/off", handleForm);
  server.on("/p", handleForm);
  server.on("/x", handleForm);

  server.begin();
}
