#include <FastLED.h>

#define NUM_LEDS 24
#define DATA_PIN  7

#define RADIUS 1.8

CRGB leds[NUM_LEDS];

uint32_t cx, cy, cz;

void setup() {

  Serial.begin(115200);

  LEDS.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  //LEDS.setBrightness(150);

  random16_set_seed(8934);
  random16_add_entropy(analogRead(3));

  //cX van de 0 a 65k
  cx = random16();
  cy = random16();
  cz = random16();

}


void loop() {

  //rotation va de 0 a 65k
  static uint16_t rotation = 0;

  static uint8_t hue = 0;

  EVERY_N_MILLISECONDS(25) {
    rotation += 55;
  }

  EVERY_N_MILLISECONDS(110) {
    hue++;
  }

  EVERY_N_MILLISECONDS(50) {
    cz += 80;
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
    int32_t x = cx + cos16(ledAngle) * RADIUS;
    int32_t y = cy + sin16(ledAngle) * RADIUS;

    //noise va de 0 a 0xFFFF, así que lo ajustamos a un uint_8
    uint8_t noiseH = inoise16(x, y, cz) >> 8;
    uint8_t noiseV = inoise16(y, x, cz) >> 8;

    leds[pixel] = CHSV(noiseH + hue, 255, noiseV);

  }

  LEDS.show();

}
