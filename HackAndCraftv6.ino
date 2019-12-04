#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

const boolean useWifi = true;

#define LED_PIN 2

#define LEDS_PER_PUCK 9

const uint16_t NUM_LETTERS = 10;
uint32_t pucksPerLetter[NUM_LETTERS] = {2,1,1,2,1,2,1,1,1,1};
const uint16_t NUM_PUCKS = 13;

uint16_t NUM_LEDS = LEDS_PER_PUCK * NUM_PUCKS;


#define BRIGHTNESS 160
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;
uint8_t w = 0;
uint8_t wipeDelay = 0;
uint8_t numberToWriteTo = 0;
uint8_t numberToWriteFrom = 0;
uint32_t customColour = strip.Color(r, g, b, w);
uint32_t black = strip.Color(0, 0, 0, 0);
uint32_t orange = strip.Color(200, 35, 0, 35);
uint32_t red = strip.Color(255, 0, 0, 0);
uint32_t green = strip.Color(0, 255, 0, 0);
uint32_t white = strip.Color(0, 0, 0, 255);
uint32_t lightWhite = strip.Color(0, 0, 0, 50);
uint8_t fadeFrom = strip.Color(0, 0, 0, 50);
uint32_t nextFadeTo = strip.Color(0, 0, 0, 255);
//uint32_t cycleColours[3] = {orange,red,white};
//uint32_t customCycleColours[3] = {orange,red,white};

bool cycling = false;
uint8_t currentCycleOffset = 0;
uint8_t cycleColourNumber = 0;

//interval for LEDs
unsigned long ledLastFrameMillis = 0;
unsigned long ledFrameInterval = 250; 
unsigned long currentMillis = 0; 


void setup() {
  Serial.begin(115200);
  
  strip.setBrightness(BRIGHTNESS);
  strip.begin();

  //strip.show();
  if (useWifi) {
    serverSetup();
  }
}

void loop() {
//  if (cycling && enoughMillisPassed(ledFrameInterval, ledLastFrameMillis)) {
//    setLEDsFromCycle();
//  }
  fade(orange, white, 7000, false, 0, NUM_LEDS-1);
  waitWithServer(5000);
  fade(white, orange, 7000, false, 0, NUM_LEDS-1);
  waitWithServer(5000);
  
  if (useWifi) {
    serverLoop();
  }
}

void waitWithServer(long delayTime) {
  long startMillis = millis();
  while (millis() - startMillis <= delayTime) {
    if (useWifi) {
      serverLoop();
    }
  }
}

void colorWipe(uint32_t c, uint16_t from, uint16_t to, uint8_t wait) {
  uint8_t elem = 0;
  uint16_t fromTo = abs(from - to);
  Serial.println(from);
  Serial.println(to);
  Serial.println(fromTo);
  Serial.println("****");
  for(uint16_t i=0; i<=fromTo; i++) {
    elem = i + from;
    if (from > to) {
      elem = from - i;
    }
    strip.setPixelColor(elem, c);
    if (wait > 0) {
      strip.show();
      delay(wait);
    }
    Serial.println(elem);
  }
  strip.show();
  Serial.println("Done write");
}

//void setLEDsFromCycle() {
//  currentCycleOffset++;
//  if (currentCycleOffset>2) {
//    currentCycleOffset=0;
//  }
//  for(uint16_t i=0; i<NUM_LEDS; i++) {
//    cycleColourNumber = (i+currentCycleOffset) % 3;
//    strip.setPixelColor(i, customCycleColours[cycleColourNumber]);
//  }
//  strip.show();
//}

bool enoughMillisPassed(unsigned long interval, unsigned long &lastTime) {
  currentMillis = millis();
  if (currentMillis - lastTime >= interval) {
    lastTime = currentMillis;
    return true;
  }
  return false;
}

void writeLetter(int letterNumber) {
  numberToWriteFrom = 0;
  for(uint16_t i=0; i<letterNumber; i++) {
    numberToWriteFrom += pucksPerLetter[i] * LEDS_PER_PUCK;
  }
  numberToWriteTo = (numberToWriteFrom + pucksPerLetter[letterNumber] * LEDS_PER_PUCK) - 1;
}

void fade(uint32_t cFrom, uint32_t cTo, long timeForFade, boolean sweep, uint16_t from, uint16_t to) {
  nextFadeTo = cTo;
  uint8_t rStart = getred(cFrom);
  uint8_t gStart = getgreen(cFrom);
  uint8_t bStart = getblue(cFrom);
  uint8_t wStart = getwhite(cFrom);
  int rDiff = getColourDiff(cFrom, cTo, 'r');
  int gDiff = getColourDiff(cFrom, cTo, 'g');
  int bDiff = getColourDiff(cFrom, cTo, 'b');
  int wDiff = getColourDiff(cFrom, cTo, 'w');
  int steps = 100;
  long delayVal = timeForFade / steps;
  if (sweep) {
    steps = steps*NUM_LEDS;
    delayVal = delayVal/NUM_LEDS;
  }
  float rInc = (float) rDiff / steps;
  float gInc = (float) gDiff / steps;
  float bInc = (float) bDiff / steps;
  float wInc = (float) wDiff / steps;
  for(uint16_t stepNo=0; stepNo<=steps; stepNo++) {
    r = (uint8_t) rStart + (stepNo * rInc);
    g = (uint8_t) gStart + (stepNo * gInc);
    b = (uint8_t) bStart + (stepNo * bInc);
    w = (uint8_t) wStart + (stepNo * wInc);
    customColour = strip.Color(r, g, b, w);
    for(uint16_t i=min(from,to); i<=max(from,to); i++) {
      if (sweep) {
        r = (uint8_t) rStart + ((stepNo - i) * rInc);
        g = (uint8_t) gStart + ((stepNo - i) * gInc);
        b = (uint8_t) bStart + ((stepNo - i) * bInc);
        w = (uint8_t) wStart + ((stepNo - i) * wInc);
        customColour = strip.Color(r, g, b, w);
      }
      strip.setPixelColor(i, customColour);
    }
    strip.show();
    delay(delayVal);
  }
}

int getColourDiff(uint32_t cFrom, uint32_t cTo, char colourPart) {
  uint8_t componentFrom;
  uint8_t componentTo;
  if (colourPart == 'r') {
    componentFrom = getred(cFrom);
    componentTo = getred(cTo);
  }
  if (colourPart == 'g') {
    componentFrom = getgreen(cFrom);
    componentTo = getgreen(cTo);
  }
  if (colourPart == 'b') {
    componentFrom = getblue(cFrom);
    componentTo = getblue(cTo);
  }
  if (colourPart == 'w') {
    componentFrom = getwhite(cFrom);
    componentTo = getwhite(cTo);
  }
  
  int diff = abs(componentFrom - componentTo);
  if (componentTo < componentFrom) {
    diff = diff * -1;
  }
  return diff;
}

uint8_t getred(uint32_t c) {
  return (c >> 16);
}
uint8_t getgreen(uint32_t c) {
  return (c >> 8);
}
uint8_t getblue(uint32_t c) {
  return (c);
}
uint8_t getwhite(uint32_t c) {
  return (c >> 24);
}
