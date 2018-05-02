#include <FastLED.h>

#define NUM_LEDS 24
#define DATA_PIN  7

#define RADIUS 3

CRGB leds[NUM_LEDS];

uint16_t x, y, z;

void setup() {

  Serial.begin(115200);

  LEDS.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  //LEDS.setBrightness(150);

  random16_set_seed(8934);
  random16_add_entropy(analogRead(3));

  x = random8() << 8;
  y = random8() << 8;
  z = random8();

}


void loop() {

  static uint8_t ihue = 0, rotation = 0;

  EVERY_N_MILLISECONDS( 70 ) {
    ihue++;
  }

  EVERY_N_MILLISECONDS( 90 ) {
    rotation++;
  }

  EVERY_N_MILLISECONDS( 50 ) {
    z += 1;
  }


  for (int pixel = 0; pixel < NUM_LEDS; pixel++) {
    uint16_t angle = map(pixel, 0, NUM_LEDS, 0, 255);
    uint16_t cx = x + cos8(angle + rotation) * RADIUS;
    uint16_t cy = y + sin8(angle + rotation) * RADIUS;
    leds[pixel] = CHSV(ihue + (inoise8(cx, cy, z)), 255, 255/* inoise8(cy, cx)*/);
  }

  LEDS.show();
  LEDS.delay(20);

}

