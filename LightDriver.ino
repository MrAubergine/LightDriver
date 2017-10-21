#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define NUM_STRIPS 4
#define NUM_LEDS_STRIP1 24
#define NUM_LEDS_STRIP2 78
#define NUM_LEDS_STRIP3 90
#define NUM_LEDS_STRIP4 79
#define NUM_LEDS NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2 + NUM_LEDS_STRIP3 + NUM_LEDS_STRIP4

CRGB leds[NUM_LEDS];

#define BRIGHTNESS         10
#define FRAMES_PER_SECOND  120

void printfunc(char *fmt, ...) {
	char buf[128]; // resulting string limited to 128 chars
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 128, fmt, args);
	va_end(args);
	Serial.print(buf);
}

void setup() {
  delay(1000); // 3 second delay for recovery
  
  FastLED.addLeds<NEOPIXEL, 8>(leds, 0, NUM_LEDS_STRIP1);

  FastLED.addLeds<NEOPIXEL, 9>(leds, NUM_LEDS_STRIP1, NUM_LEDS_STRIP2);

  FastLED.addLeds<NEOPIXEL, 10>(leds, NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2, NUM_LEDS_STRIP3);

  FastLED.addLeds<NEOPIXEL, 11>(leds, NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2 + NUM_LEDS_STRIP3, NUM_LEDS_STRIP4);
  
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(38400);
  Serial1.begin(9600);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
bool gComms = false;

void loop()
{
	if (gComms)
	{
		commloop();
	}
	else
	{
		// Call the current pattern function once, updating the 'leds' array
		gPatterns[gCurrentPatternNumber]();

		// send the 'leds' array out to the actual LED strip
		FastLED.show();
		// insert a delay to keep the framerate modest
		FastLED.delay(1000 / FRAMES_PER_SECOND);

		// do some periodic updates
		EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
											  // EVERY_N_SECONDS(10) { nextPattern(); Serial.write("Changing\n");  } // change patterns periodically

		if (Serial1.available() > 0)
		{
			if (Serial1.read() == '*')
			{
				Serial1.write('R');
				gComms = true;
			}
		}
	}
}

char inbuf[12];

void commloop()
{

	// Early out if there's nothing to read
	if (Serial1.available() == 0)
		return;

	Serial.print("Bloop\n");
}

void processinput()
{
	Serial.write("Received:");
	Serial.flush();
	Serial.write(inbuf, 10);
	Serial.flush();
	Serial.write("qq\n");
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
//  fill_solid(leds, NUM_LEDS, CRGB::DarkOrange);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

