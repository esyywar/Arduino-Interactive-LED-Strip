// FastLED library definitions
#include <FastLED.h>
#define LED_PIN     5
#define NUM_LEDS    12
#define BRIGHTNESS  64
#define CONTROLS    3
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

// Number of LEDs to update based on live sound (might prefer change based on strip length)
#define MUSIC_LEDS 1

// interval of time for music visualizer to increment colour
#define MUSIC_CLR_INTERVAL 100

// update rate
#define UPDATES_PER_SECOND 100

// Number of LEDs to update based on live sound (might prefer change based on strip length)
// const int musicLEDs = MUSIC_LEDS;
const int clrInterval = MUSIC_CLR_INTERVAL;

uint8_t colour = 1;

int originPin = A0;  // potentio pin to set origin LED on the strip for music visualization
int threshPin = A1;  // potentio pin for threhold value 
int clrIntervalPin = A2;   // potentiometer pin for length of MA
int sensorPin = A3;    // read value from the sound sensor
int sensorValue;  // variable to store the value coming from the sensor

unsigned long lastClrChange;


void setup() {

  // FastLED library set-up
  FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(64);
  
  // data rate
  Serial.begin(9600);
}


void loop() {
  int origin = map(analogRead(originPin), 0, 1023, 0, NUM_LEDS); 
  int threshold = analogRead(threshPin);
  int clrInterval = map(analogRead(clrIntervalPin), 0, 1023, 0, 500);
  
  // increment the colour every specified amount of time
  if (millis() - lastClrChange > clrInterval)
  {
    colour ++;
    lastClrChange = millis();
  }

  // read sensor value that is averaged over specified number of data points
  sensorValue = analogRead(sensorPin);
  
  // colour the first pixel according to the live sound
  colourOriginPixel(sensorValue, colour, threshold, origin);  

  // propagate the colour wave in 2 directions from origin
  pointSourceWave(origin);
  //moveWave();

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}


// colour pixels at the chosen source on LED strip
void colourOriginPixel (int sensorValue, uint8_t colour, int threshold, int origin) {
  uint8_t brightness;

  int farEndSource = origin + MUSIC_LEDS - 1;
  int nearEndSource = origin - MUSIC_LEDS - 1;

  for (int i = nearEndSource; i < farEndSource; i++) {
    if (sensorValue > threshold)
    {
      brightness = map(sensorValue, 0, 1023, 80, 255);
      leds[i] = CHSV(colour, 255, brightness);
    }
    else
    {
      leds[i].setRGB(0, 0, 0);
    }
  }
}


// function to move the wave from first LED toward the far end
void moveWave() {
  int blanks = MUSIC_LEDS - 1;
  
  for (int i = NUM_LEDS; i > blanks; i--)
  {
    leds[i] = leds[i - 1];
  }
}


// function takes an origin point LED and propagates the wave in both directions away from source
void pointSourceWave (int origin) {
  // identify which LED to start the wave from (subtract 1 for indexing beginning at 0)
  int farEndStart = origin + MUSIC_LEDS - 2;
  int nearEndStart = origin - MUSIC_LEDS - 1;

  // move LED wave toward the far end
  for (int i = NUM_LEDS; i > farEndStart; i--)
  {
    leds[i] = leds[i - 1];
  }

  // move LED wave toward the near end
  for (int i = 0; i < nearEndStart; i++)
  {
    leds[i] = leds[i + 1];    
  }
}
