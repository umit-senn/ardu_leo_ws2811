#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    (60*5)
#define BRIGHTNESS  60  // %35

// Yenileme araligi (50 Hz)
#define FRAME_MS    20

CRGB leds[NUM_LEDS];

// Nefes alma animasyonu: tum ledler verilen renkte yumusakca yanip soner.
//   color    : renk, ornegin CRGB::Red, CRGB(255, 100, 0), CHSV(160, 255, 255)
//   periodMs : bir tam nefesin (yan + son) suresi, milisaniye
//   minLevel : en sonuk andaki parlaklik (0 = tamamen soner, 255 = hic sonmez)
void breathing(CRGB color, uint16_t periodMs = 4000, uint8_t minLevel = 10)
{
    // 0..255 arasi faz, periodMs surede bir tur
    uint8_t phase = (uint32_t)(millis() % periodMs) * 256UL / periodMs;
    uint8_t wave  = quadwave8(phase);                         // 0..255..0 yumusak dalga
    uint8_t level = minLevel + scale8(wave, 255 - minLevel);  // minLevel..255

    color.nscale8_video(level);  // rengin tonunu koruyarak parlakligini ayarla
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(FRAME_MS);
}

void setup()
{
    FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setDither(0);   // dusuk parlaklikta titremeyi onler
    FastLED.clear(true);
}

void loop()
{
    breathing(CRGB::Red);

    // Diger ornekler:
    // breathing(CRGB::Blue);                 // mavi, varsayilan hiz
    // breathing(CRGB(255, 80, 0), 6000);     // turuncu, 6 saniyelik yavas nefes
    // breathing(CRGB::Purple, 2000, 0);      // mor, hizli, tamamen sonerek
}
