#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    (60*5)
#define BRIGHTNESS  80  // %35

// Yenileme araligi (50 Hz)
#define FRAME_MS    20

// Her animasyonun suresi (saniye). Sure dolunca bir sonrakine yumusak gecis yapilir.
#define ANIM_DURATION_S  20

// Sadece tek bir animasyon oynatmak icin listedeki sirasini yaz (0, 1, 2 ...).
// -1 = hepsini sirayla oynat.
#define FIXED_ANIMATION  -1

CRGB leds[NUM_LEDS];

// =====================================================================
//  Animasyonlar: her fonksiyon sadece bir kare cizer, show() loop'ta yapilir
// =====================================================================

// Nefes alma: tum ledler verilen renkte yumusakca yanip soner.
//   color    : renk, ornegin CRGB::Red, CRGB(255, 100, 0), CHSV(160, 255, 255)
//   periodMs : bir tam nefesin (yan + son) suresi, milisaniye
//   minLevel : en sonuk andaki parlaklik (0 = tamamen soner, 255 = hic sonmez)
void breathing(CRGB color, uint16_t periodMs = 4000, uint8_t minLevel = 10)
{
    uint8_t phase = (uint32_t)(millis() % periodMs) * 256UL / periodMs;
    uint8_t wave  = quadwave8(phase);                         // 0..255..0 yumusak dalga
    uint8_t level = minLevel + scale8(wave, 255 - minLevel);  // minLevel..255

    color.nscale8_video(level);  // rengin tonunu koruyarak parlakligini ayarla
    fill_solid(leds, NUM_LEDS, color);
}

// Renk degistiren nefes: renk her nefeste yavasca bir sonrakine kayar
void breathingRainbow()
{
    uint8_t hue = millis() / 100;  // ~25 saniyede renk tekerleginin tam turu
    breathing(CHSV(hue, 255, 255), 5000, 5);
}

// Bir kalp vurusu: aniden parlar, hizla soner
static uint8_t pulse(int16_t x)
{
    if (x < 0 || x >= 300) return 0;
    uint8_t v = 255 - (uint32_t)x * 255 / 300;
    return scale8(v, v);  // karesini al: daha keskin sonus
}

// Kalp atisi: kirmizi "lup-dup" cift vurus, sonra dinlenme
void heartbeat()
{
    int16_t t = millis() % 1200;  // ~50 nabiz/dakika, sakin bir kalp
    uint8_t level = max(pulse(t), scale8(pulse(t - 280), 170));  // ikinci vurus biraz zayif
    if (level < 8) level = 8;                                    // tamamen kararmasin
    fill_solid(leds, NUM_LEDS, CRGB(level, 0, 0));
}

// Kayan gokkusagi
void rainbowFlow()
{
    static uint8_t hue = 0;
    hue++;
    fill_rainbow(leds, NUM_LEDS, hue, max(1, 255 / NUM_LEDS));
}

// Kuyruklu yildiz: serit boyunca gidip gelen, rengi degisen, kivilcim birakan isik
void comet()
{
    static int16_t pos = 0;  // 8.8 sabit noktali konum
    static int8_t  dir = 1;
    static uint8_t hue = 0;

    // Kuyrugu dagitarak sondur: bazi pikseller daha yavas soner, kivilcim etkisi
    for (int i = 0; i < NUM_LEDS; i++) {
        if (random8() < 180) leds[i].fadeToBlackBy(50);
    }

    pos += dir * 140;
    int head = pos >> 8;
    if (head >= NUM_LEDS - 1) { head = NUM_LEDS - 1; pos = head << 8; dir = -1; }
    if (head <= 0)            { head = 0;            pos = 0;         dir = 1;  }

    hue++;
    leds[head] = CHSV(hue, 200, 255);
}

// Ates: sicaklik simulasyonu, seridin basindan sonuna yanan alevler
void fire()
{
    static uint8_t heat[NUM_LEDS];
    const uint8_t COOLING  = 55;
    const uint8_t SPARKING = 120;

    // 1) Her hucre biraz soguyor
    for (int i = 0; i < NUM_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
    // 2) Isi ileri dogru yayiliyor
    for (int k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }
    // 3) Basta rastgele yeni kivilcimlar
    if (random8() < SPARKING) {
        int y = random8(min(7, NUM_LEDS));
        heat[y] = qadd8(heat[y], random8(160, 255));
    }
    // 4) Sicakligi renge cevir
    for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = HeatColor(heat[j]);
    }
}

// Yildizli gece: koyu lacivert zeminde rastgele parlayip sonen yildizlar
void twinkleStars()
{
    const CRGB sky(0, 0, 6);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i].fadeToBlackBy(12);
        leds[i] |= sky;  // zemin rengi hep kalsin
    }
    if (random8() < 90) {
        int i = random16(NUM_LEDS);
        // cogu sicak beyaz, arada bir renkli yildiz
        if (random8() < 200) leds[i] = CRGB(255, 180, 90);
        else                 leds[i] = CHSV(random8(), 180, 255);
    }
}

// Kutup isiklari: yesil, turkuaz ve mor tonlarinda yavasca dalgalanan perde
void aurora()
{
    static const CRGBPalette16 pal(CRGB(0, 8, 4), CRGB(0, 160, 60),
                                   CRGB(0, 110, 150), CRGB(110, 0, 140));
    uint32_t t = millis();
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t idx = inoise8(i * 25, t / 12);        // renk dalgasi
        uint8_t bri = inoise8(i * 15 + 5000, t / 8);  // parlaklik dalgasi
        bri = qadd8(qsub8(bri, 60), qsub8(bri, 60));  // koyu bosluklar olustur
        leds[i] = ColorFromPalette(pal, idx, bri);
    }
}

// Okyanus: mavi ve turkuaz dalgalar
void ocean()
{
    uint32_t t = millis();
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t idx = inoise8(i * 40, t / 6);
        uint8_t bri = beatsin8(12, 120, 255, 0, i * 6);  // seritte ilerleyen dalga
        leds[i] = ColorFromPalette(OceanColors_p, idx, bri);
    }
}

// =====================================================================
//  Animasyon listesi
// =====================================================================

void breathingRed() { breathing(CRGB::Red); }

typedef void (*Animation)();
Animation animations[] = {
    breathingRed,       // 0  kirmizi nefes
    breathingRainbow,   // 1  renk degistiren nefes
    heartbeat,          // 2  kalp atisi
    rainbowFlow,        // 3  kayan gokkusagi
    comet,              // 4  kuyruklu yildiz
    fire,               // 5  ates
    twinkleStars,       // 6  yildizli gece
    aurora,             // 7  kutup isiklari
    ocean,              // 8  okyanus
};
const uint8_t NUM_ANIMATIONS = sizeof(animations) / sizeof(animations[0]);

uint8_t  current   = (FIXED_ANIMATION >= 0) ? FIXED_ANIMATION : 0;
uint32_t animStart = 0;

// Gecis: mevcut goruntuyu yarim saniyede karartir
void fadeOut()
{
    for (int k = 0; k < 25; k++) {
        fadeToBlackBy(leds, NUM_LEDS, 30);
        FastLED.show();
        delay(FRAME_MS);
    }
    FastLED.clear();
}

void setup()
{
    FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setDither(0);   // dusuk parlaklikta titremeyi onler
    FastLED.clear(true);
    random16_add_entropy(analogRead(A0));
}

void loop()
{
    if (FIXED_ANIMATION < 0 && millis() - animStart > ANIM_DURATION_S * 1000UL) {
        fadeOut();
        current = (current + 1) % NUM_ANIMATIONS;
        animStart = millis();
    }

    animations[current]();
    FastLED.show();
    delay(FRAME_MS);
}
