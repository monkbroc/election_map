#include "Adafruit_mfGFX/Adafruit_mfGFX.h"   // Core graphics library
#include "RGBmatrixPanel/RGBmatrixPanel.h" // Hardware-specific library
#include "blynk/blynk.h"
#include <string.h>
#include <stdio.h>

#define STATES 50

enum {
  BLANK,
  DEMOCRAT,
  REPUBLICAN
};

#define DEMOCRAT_COUNT_ADDR 200
#define REPUBLICAN_COUNT_ADDR 202

uint16_t democratCount = 0;
uint16_t republicanCount = 0;

uint8_t xoffset = 6;
uint8_t yoffset = 8;

// Blynk transport
const char auth[] = "7e0a285717c04d6a8528d2af74287aec";

// RGB shield
	#define CLK	D6
	#define OE	D7
	#define LAT	TX
	#define A  	A0
	#define B  	A1
	#define C  	A2
	#define D	RX
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

int setState(String arg) {
    char strBuffer[125];
    arg.toCharArray(strBuffer, 125);
    uint8_t state = (uint8_t)atoi(strtok(strBuffer, ","));
    uint8_t x = (uint8_t)atoi(strtok(NULL, ","));
    uint8_t y = (uint8_t)atoi(strtok(NULL, ","));
  
    int addr = 3 * state;
    EEPROM.write(addr, 0);
    EEPROM.write(addr + 1, x);
    EEPROM.write(addr + 2, y);

    return 0;
}

void saveCount() {
  EEPROM.put(DEMOCRAT_COUNT_ADDR, democratCount);
  EEPROM.put(REPUBLICAN_COUNT_ADDR, republicanCount);
}

int clearMap(String) {
  for (int state = 0; state < STATES; state++) {
    int addr = 3 * state;
    EEPROM.write(addr, 0);
  }

  republicanCount = 0;
  democratCount = 0;
  saveCount();

  return 0;
}

void setup() {
  Particle.function("state", setState);
  Particle.function("clear", clearMap);

  EEPROM.get(DEMOCRAT_COUNT_ADDR, democratCount);
  EEPROM.get(REPUBLICAN_COUNT_ADDR, republicanCount);

  Blynk.begin(auth);
  matrix.begin();
}

void displayPanel() {
  for (int state = 0; state < STATES; state++) {
    int addr = 3 * state;
    uint8_t vote = EEPROM.read(addr);
    uint8_t x = EEPROM.read(addr + 1);
    uint8_t y = EEPROM.read(addr + 2);

    int color = 0;
    switch (vote) {
      case BLANK: color = matrix.Color888(0xFF, 0xFF, 0xFF); break;
      case DEMOCRAT: color = matrix.Color888(0, 0, 0xFF); break;
      case REPUBLICAN: color = matrix.Color888(0xFF, 0, 0); break;
    }

    if (vote >= 0 && vote <= 2) {
      uint8_t a = xoffset + 2*x;
      uint8_t b = yoffset + 2*y;
      matrix.drawPixel(a, b, color);
      matrix.drawPixel(a+1, b, color);
      matrix.drawPixel(a, b+1, color);
      matrix.drawPixel(a+1, b+1, color);
    }
  }
}

void loop()
{
  Blynk.run();
  displayPanel();
}

uint8_t state = 0;
BLYNK_WRITE(V0) // Blynk V0 is state
{
  state = (uint8_t) param.asInt() - 1;
}

BLYNK_WRITE(V1) // Blynk V1 is vote democrat
{
  int addr = state * 3;
  EEPROM.write(addr, DEMOCRAT);
  democratCount += 1;
  saveCount();
}

BLYNK_WRITE(V2) // Blynk V2 is vote republican
{
  int addr = state * 3;
  EEPROM.write(addr, REPUBLICAN);
  republicanCount += 1;
  saveCount();
}

BLYNK_READ(V3) // Blynk V3 is for democrat count
{
  Blynk.virtualWrite(V3, democratCount);
}

BLYNK_READ(V4) // Blynk V4 is for democrat count
{
  Blynk.virtualWrite(V4, republicanCount);
}
