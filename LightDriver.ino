#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define NUM_STRIPS 4
#define NUM_LEDS_STRIP1 93
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

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
bool gComms = false;
unsigned long gStartComms = 0;

void loop()
{
	if (gComms)
	{
		commloop();
	}
	else
	{
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
				gStartComms = millis();
				gComms = true;
			}
		}
	}
}

void commloop()
{
	// strip off any remaining *s
	while (Serial1.peek() == '*')
		Serial1.read();

	// wait until we have 8 bytes read or 500ms has passed
	if (Serial1.available() >= 8)
	{
		char cmd[9];
		Serial1.readBytes(cmd, 8);
		handleinput(cmd);
		cmd[8] = 0;
		Serial.println(cmd);
		gComms = false;
	}
	else if (millis() > gStartComms + 500)
	{
		Serial.println("timeout");
		gComms = false;
	}
}

void handleinput(char* cmd)
{
	char celem[3];
	celem[2] = 0;
	celem[0] = cmd[2];
	celem[1] = cmd[3];
	int r = strtoul(celem, NULL, 16);
	celem[0] = cmd[4];
	celem[1] = cmd[5];
	int g = strtoul(celem, NULL, 16);
	celem[0] = cmd[6];
	celem[1] = cmd[7];
	int b = strtoul(celem, NULL, 16);

	CRGB col = CRGB(r, g, b);
	fill_solid(leds, NUM_LEDS, col);
}


