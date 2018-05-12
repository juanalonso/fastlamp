//https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html

#define FASTLED_ESP8266_D1_PIN_ORDER
#include <FastLED.h>
#define NUM_LEDS 24
#define DATA_PIN D8

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

const char *ssid = "Fastlamp";
const char *password = "cucaracha";
const char *mdnsName = "lamp";

enum potType {
  ROTATION, HUE, Z, RADIUS, BRIGHTNESS, SATURATION
};

CRGB leds[NUM_LEDS];
uint32_t cx, cy, cz;

ESP8266WebServer server(80);
//WebSocketsServer webSocket = WebSocketsServer(81);


void setup() {

  initSerial();
  initLeds();
  initRandomValues();
  initSoftAP();
  initmDNS();
  initWebServer();
  //initWebsocket();

  pinMode(LED_BUILTIN, OUTPUT);
  for (int f = 0; f < 10; f++) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
  }

}


void loop() {

  server.handleClient();

  /*

    for (int f = 0; f < 6; f++) {
    analogValue[f] = 1023 - analogRead(f);
    }

    //rotation va de 0 a 65k
    static uint16_t rotation = 0;

    static uint8_t hue = 0;

    EVERY_N_MILLISECONDS(25) {
    rotation += map(analogValue[ROTATION], 0, 1000, 0, 1750);
    }

    EVERY_N_MILLISECONDS(100) {
    hue += map(analogValue[HUE], 0, 1000, 0, 5);
    }

    EVERY_N_MILLISECONDS(50) {
    cz += map(analogValue[Z], 0, 1000, 0, 2500);
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

    leds[pixel] = CHSV(noiseH + hue, map(analogValue[SATURATION], 0, 1000, 0, 250), 255);

    }

    LEDS.show();
  */

}
/*
  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        //rainbow = false;                  // Turn rainbow off when a new connection is established
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
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
  }
*/

void initSerial() {
  Serial.begin(115200);
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
}

void initSoftAP() {
  WiFi.softAP(ssid, password);
  Serial.print("Access point: ");
  Serial.println(ssid);
  Serial.print("  IP address: ");
  Serial.println(WiFi.softAPIP());
}

void initmDNS() {
  if (!MDNS.begin(mdnsName)) {
    Serial.println("Error setting up mDNS responder!");
  }
  Serial.println("mDNS responder started");
}

void initWebServer() {
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void initWebsocket() {
  //webSocket.begin();
  //webSocket.onEvent(webSocketEvent);
}

void handleRoot() {
  server.send(200, "text/html", "<a href='/toggle'>Toggle LED</a>");
}

void handleToggle() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

