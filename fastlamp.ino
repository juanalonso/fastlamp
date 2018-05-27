//https://github.com/esp8266/arduino-esp8266fs-plugin
//https://bulma.io

//https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>
#define NUM_LEDS 24
#define DATA_PIN 4

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsServer.h>

const char *ssid = "Fastlamp";
const char *password = "cucaracha";
const char *mdnsName = "lamp";

enum potType {
  ROTATION, HUE, DELTAZ, RADIUS, BRIGHTNESS, SATURATION
};

CRGB leds[NUM_LEDS];
uint32_t cx, cy, cz;
int analogValue[6];
boolean isStoppedHue = false;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);


void setup() {

  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);

  initSerial();
  initSPIFFS();
  initLeds();
  initRandomValues();
  initSoftAP();
  initmDNS();
  initWebServer();
  initWebsocket();

  //digitalWrite(LED_BUILTIN, HIGH);

}


void loop() {

  webSocket.loop();
  server.handleClient();

  //rotation va de 0x00 a 0xFFFF
  static uint16_t rotation = 0;

  //hue va de 0x00 a 0xFF
  static uint8_t hue = 0;

  EVERY_N_MILLISECONDS(25) {
    rotation += map(analogValue[ROTATION], 0, 1000, 0, 1750);
  }

  EVERY_N_MILLISECONDS(100) {
    if (!isStoppedHue) {
      hue += map(analogValue[HUE], 0, 1000, 0, 5);
    }
  }

  EVERY_N_MILLISECONDS(50) {
    cz += map(analogValue[DELTAZ], 0, 1000, 0, 2500);
  }

  EVERY_N_MILLISECONDS(100) {
    LEDS.setBrightness(map(analogValue[BRIGHTNESS], 0, 1020, 5, 250));
  }

  for (int pixel = 0; pixel < NUM_LEDS; pixel++) {

    //ledAngle convierte 0..NUM_LEDS en 0..0xFFFF
    //y así ya tenemos rotation y ledAngle en la misma escala
    //La suma hace bien el desbordamiento.
    uint16_t ledAngle = map(pixel, 0, NUM_LEDS, 0, 0xffff) + rotation;

    //cos16 devuelve un valor entre -32k y 32k.
    //Como hay que sumar y escalar, lo convertimos en un int32.
    //OJO: para que la escala sea correcta, RADIUS siempre ha de llevar punto decimal
    //No tengo muy claro por qué si pongo uint en vez de int,
    //x a veces genera una onda cuadrada
    int32_t x = cx + cos16(ledAngle) * map(analogValue[RADIUS], 0, 1000, 0, 500) / 100.0;
    int32_t y = cy + sin16(ledAngle) * map(analogValue[RADIUS], 0, 1000, 0, 500) / 100.0;

    //noise va de 0 a 0xFFFF, así que lo ajustamos a un uint_8
    uint8_t noiseH = inoise16(x, y, cz) >> 8;
    uint8_t noiseV = inoise16(y, x, cz) >> 8;

    leds[pixel] = CHSV(noiseH + hue, 255, 255);
    //map(analogValue[SATURATION], 0, 1000, 0, 250);

  }

  LEDS.show();


}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  Serial.print("Evento ");
  switch (type) {

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WS    Connected from %d.%d.%d.%d url: %s\n", ip[0], ip[1], ip[2], ip[3], payload);
        break;
      }

    case WStype_TEXT:
      if (payload[0] == '#' && lenght >= 4 && payload[1] >= '1' && payload[1] <= '6') {
        uint16_t value = (uint16_t) strtol((const char *) &payload[3], NULL, 10);
        analogValue[payload[1] - 49] = value;
      } else {
        Serial.println("WStype_TEXT");
        Serial.printf("    %s\n", payload);
      }
      break;
    case WStype_ERROR:
      Serial.println("WS .   WStype_ERROR");
      break;
    case WStype_DISCONNECTED:
      Serial.println("WS    Disconnected\n");
      break;
  }



  /*
    switch (type) {

    case WStype_TEXT:                     // if new text data is received
    Serial.printf("[ %u] get Text: %s\n", num, payload);
    if (payload[0] == '#') {            // we get RGB data
    uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);   // decode rgb data
    int r = ((rgb >> 20) & 0x3FF);                     // 10 bits per color, so R: bits 20-29
    int g = ((rgb >> 10) & 0x3FF);                     // G: bits 10-19
    int b =          rgb & 0x3FF;                      // B: bits  0-9

    //analogWrite(LED_RED,   r);                         // write it to the LED output pins
    //analogWrite(LED_GREEN, g);
    //analogWrite(LED_BLUE,  b);
    } else if (payload[0] == 'R') {                      // the browser sends an R when the rainbow effect is enabled
    //rainbow = true;
    } else if (payload[0] == 'N') {                      // the browser sends an N when the rainbow effect is disabled
    //rainbow = false;
    }
    break;
    }
  */
}


void initSerial() {
  Serial.begin(115200);
}


void initSPIFFS() {
  SPIFFS.begin();
  Serial.println("File list: ");
  String str = "";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    str += "    ";
    str += dir.fileName();
    str += " / ";
    str += dir.fileSize();
    str += "\r\n";
  }
  Serial.print(str);
}


void initLeds() {
  LEDS.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  LEDS.setBrightness(0);
}


void initRandomValues() {

  random16_set_seed(8934);
  random16_add_entropy(analogRead(A0));
  cx = random16();
  cy = random16();
  cz = random16();

  analogValue[ROTATION] = random(500);
  analogValue[HUE] = random(500);
  analogValue[DELTAZ] = random(500);
  analogValue[RADIUS] = random(500);
  analogValue[BRIGHTNESS] = 1000;
  analogValue[SATURATION] = 1023;

  isStoppedHue = false;

}


void initSoftAP() {
  WiFi.softAP(ssid, password);
  Serial.print("    Access point: ");
  Serial.println(ssid);
  Serial.print("      IP address: ");
  Serial.println(WiFi.softAPIP());
}

void initmDNS() {
  if (!MDNS.begin(mdnsName)) {
    Serial.println("*** Error setting up mDNS responder!");
  }
  Serial.println("  mDNS responder: OK");
}


void initWebServer() {
  server.onNotFound(handleRequests);
  server.begin();
  Serial.println("     HTTP server: OK");
}


void initWebsocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Websocket server: OK");

}


void handleRequests() {

  String filename = server.uri();

  Serial.print("Request: ");
  Serial.println(filename);

  if (filename == "/random") {
    initRandomValues();
    server.sendHeader("Location", "/");
    server.send(303);
    return;
  }

  if (filename == "/stophue") {
    isStoppedHue = !isStoppedHue;
    server.sendHeader("Location", "/");
    server.send(303);
    return;
  }

  if (!handleFileRead(filename))  {
    server.send(404, "text/plain", "404: Not found");
  }
}


bool handleFileRead(String path) {

  if (path.endsWith("/")) {
    path += "index.html";
  }

  if (!path.startsWith("/")) {
    path = "/" + path;
  }

  /*
    Serial.print("   Path: ");
    Serial.println(path);
  */

  String contentType = getContentType(path);

  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }

  Serial.println("      ** File not found");
  return false;
}


String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}


