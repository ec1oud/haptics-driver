#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <DigiCDC.h>

Adafruit_DRV2605 drv;
char buf[8];
uint8_t bufOffset = 0;
int effect = 0;

void setup()
{
	memset(buf, 0, sizeof(buf));
	SerialUSB.begin();
	SerialUSB.println("DRV2605");
	drv.begin();

	drv.selectLibrary(1);

	// I2C trigger by sending 'go' command
	// default, internal trigger when sending GO command
	drv.setMode(DRV2605_MODE_INTTRIG);
}

void loop()
{
	if (SerialUSB.available()) {
		char ch = SerialUSB.read();

		if (ch == '\r' || ch == '\n') {
			if (bufOffset > 0 && bufOffset < sizeof(buf) - 1) {
				effect = atoi(buf);
			}

			memset(buf, 0, sizeof(buf));
			bufOffset = 0;

			SerialUSB.print("\nEffect #"); SerialUSB.println(effect);
			drv.setWaveform(0, effect);			  // play effect
			drv.setWaveform(1, 0);				  // end waveform
			drv.go();
			//      SerialUSB.delay(10);
		}
		else if (bufOffset < sizeof(buf)) {
			buf[bufOffset++] = ch;
			SerialUSB.write(ch);
		}
	}
}
