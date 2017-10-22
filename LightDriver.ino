#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DEV_BOARD // define this to use the dev board, undef to use the real room lights

#define NUM_STRIPS 4
#define NUM_LEDS_STRIP1 93
#define NUM_LEDS_STRIP2 78
#define NUM_LEDS_STRIP3 90
#define NUM_LEDS_STRIP4 79
#define NUM_LEDS NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2 + NUM_LEDS_STRIP3 + NUM_LEDS_STRIP4

CRGB leds[NUM_LEDS];
uint8_t gBrightness = 0;

#ifdef DEV_BOARD
	uint8_t gBrightnessTarget = 10;
	HardwareSerial& BTSerial = Serial1;
	#define DebugPrint(x) Serial.print(x)
	#define DebugPrintLn(x) Serial.println(x)
#else
	uint8_t gBrightnessTarget = 160;
	HardwareSerial& BTSerial = Serial;
	#define DebugPrint(x)
	#define DebugPrintLn(x)
#endif

#define FRAMES_PER_SECOND  120

void setup() {
  delay(1000); // 1 second delay for recovery
  
  FastLED.addLeds<NEOPIXEL, 8>(leds, 0, NUM_LEDS_STRIP1);

  FastLED.addLeds<NEOPIXEL, 9>(leds, NUM_LEDS_STRIP1, NUM_LEDS_STRIP2);

  FastLED.addLeds<NEOPIXEL, 10>(leds, NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2, NUM_LEDS_STRIP3);

  FastLED.addLeds<NEOPIXEL, 11>(leds, NUM_LEDS_STRIP1 + NUM_LEDS_STRIP2 + NUM_LEDS_STRIP3, NUM_LEDS_STRIP4);
  
  // set master brightness control
  FastLED.setBrightness(gBrightness);

#ifdef DEV_BOARD
	Serial.begin(38400);
	BTSerial.begin(9600);
#else
	Serial.begin(9600);
#endif
}

enum
{
	ModeSolid,
	ModeRainbow,
} gMode = ModeRainbow;
int gRainbowDelta = 7;
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

		if (gMode == ModeRainbow)
		{
			fill_rainbow(leds, NUM_LEDS, gHue, gRainbowDelta);
		}

		if (gBrightness != gBrightnessTarget)
		{
			int step;
			if (abs(gBrightness - gBrightnessTarget) > 8)
				step = 8;
			else
				step = 1;

			if (gBrightness < gBrightnessTarget)
				gBrightness += step;
			else
				gBrightness -= step;
			
			FastLED.setBrightness(gBrightness);
		}

		if (BTSerial.available() > 0)
		{
			if (BTSerial.read() == '*')
			{
				DebugPrintLn("Connected...");
				BTSerial.write("RRRR",4);
				gStartComms = millis();
				gComms = true;
			}
		}
	}
}

void commloop()
{
	// strip off any remaining *s
	while (BTSerial.peek() == '*')
	{
		char chStrip = BTSerial.read();
		DebugPrint(chStrip);
	}

	// wait until we have 8 bytes read or 500ms has passed
	if (BTSerial.available() >= 7)
	{
		char cmd[9];
		BTSerial.readBytes(cmd, 7);
		handleinput(cmd);
		cmd[7] = 0;
		DebugPrintLn();
		DebugPrint("Command...");
		DebugPrintLn(cmd);
		gComms = false;
	}
	else if (millis() > gStartComms + 1000)
	{
		DebugPrintLn();
		DebugPrint("Timeout...");
		while (BTSerial.available())
		{
			char chRemain = BTSerial.read();
			DebugPrint(chRemain);
		}
		DebugPrintLn();
		gComms = false;
	}
}

void handleinput(char* cmd)
{
	char cnum[3] = { 0 };
	int val[3];
	for (int i = 0, vi = 1; i < 3; i++, vi += 2)
	{
		cnum[0] = cmd[vi];
		cnum[1] = cmd[vi + 1];
		val[i] = strtoul(cnum, NULL, 16);
	}

	CRGB col;

	switch(cmd[0])
	{
	case 'S':
		col = CRGB(val[0], val[1], val[2]);
		fill_solid(leds, NUM_LEDS, col);
		gMode = ModeSolid;
		break;
	case 'R':
		gRainbowDelta = val[0];
		gMode = ModeRainbow;
		break;
	case 'B':
		gBrightnessTarget = val[0];
		break;
	}
}


