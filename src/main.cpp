#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    60
#define BRIGHTNESS  89  // %35

CRGB leds[NUM_LEDS];

void setup()
{
    FastLED.addLeds<WS2815, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear(true);
}

void loop()
{
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(1000);

    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(1000);

    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
    delay(1000);
}