/* LED Strip Controller
 * By: Rahul Eswar
 * Date: October 19, 2019

 * This program is made for on the fly manual control of the WS2812B LED lightstrip.
 * The user can control and configure the LED settings with 3 potentiometer knobs and 1 button
 * The button works to change modes between: HSV colour set, patterned light show and music visualization
 * Knobs control different settings depending on the mode set (e.g. colour in colour set mode, frequency focus in music mode etc.)
 */

// FastLED library definitions
#include <FastLED.h>
#define LED_PIN     5
#define NUM_LEDS    12
#define BRIGHTNESS  64
#define CONTROLS    3
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// Music visualizer custom definitions
#define MUSIC_LEDS 1   // number of LEDs to update based on live sound (might prefer change based on strip length)
#define MUSIC_CLR_INTERVAL 100   // interval of time for music visualizer to increment colour

// update rate
#define UPDATES_PER_SECOND 100

// number of modes to cycle
#define NUM_MODES 3

// user input control pins
int potReadPin[] = {0, 1, 2};
const uint8_t BTN_PIN = 2;

// pin to read from sound sensor
int sensorPin = A3;

// variable to store value from sensor
int sensorValue;

// declare initial mode
uint8_t mode = 0;

// pins used for LED mode indication
const uint8_t modeLEDs[NUM_MODES] = {6, 7, 8};

// for the interrupt function
volatile byte state = NUM_MODES;
unsigned long lastInterrupt;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;


// structure for storing the 3 mapped user inputs
typedef struct userSettings {
  int inputA;
  int inputB;
  int inputC;
} userInput;


// Defining custom palette of Raptors colours!
const TProgmemPalette16 TorontoRaptors_p PROGMEM =
{
    CRGB::DarkRed,
    CRGB::DarkRed,
    CRGB::DarkRed,
    CRGB::DarkRed,
    CRGB::DarkRed,
    
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::DarkViolet,

    CRGB::Snow,
    CRGB::Snow,
    CRGB::Snow,
    CRGB::Snow,
    
    CRGB::Purple, 
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Purple   
};


// Defining custom palette of Lakers colours!
const TProgmemPalette16 Lakers_p PROGMEM =
{
    CRGB::Yellow,
    CRGB::Yellow,
    CRGB::Yellow,
    CRGB::Yellow,
    
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Purple,
    
    CRGB::White,
    CRGB::White,
    
    CRGB::Yellow,
    CRGB::Yellow,
    CRGB::Yellow,
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Purple
};


// Defining custom palette of the India flag
const TProgmemPalette16 India_p PROGMEM =
{
    CRGB::Orange,
    CRGB::Orange,
    CRGB::Gray,
    CRGB::Green,
    CRGB::Green,
    
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Gray,
    
    CRGB::Green,
    CRGB::Green,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Orange,
    CRGB::Orange,
    
    CRGB::Gray,
    CRGB::Gray
};


void setup() {
  // power-up safety delay
  delay(1500);
  
  // data rate
  Serial.begin(9600);

  // FastLED library set-up
  FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(64);

  // pin declarations for interrupt
  pinMode (BTN_PIN, INPUT_PULLUP);

  // set pins for LED mode indicators as output
  for (int i= 0; i < NUM_MODES; i++)
  {
    pinMode (modeLEDs[modeLEDs[0] + i], OUTPUT);
  }

  // attaching interrupt used for mode switch on button press
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), changeMode, FALLING);

  // initialization of colour palette color breathe mode
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;  
}


void loop() {

  // calculate the active state
  mode = state % NUM_MODES;
  Serial.println(mode);

  // change LED to show user what mode we are in
  modeIndicateLED(mode);

  // execute the operation for active mode
  executeMode(mode);

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}


// turns active mode LED on and the rest are off (indicates to user the mode we are in)
void modeIndicateLED(uint8_t mode) {
   for (int i = 0; i < NUM_MODES; i++) 
   {
      if (i == mode)
      {
        digitalWrite(modeLEDs[i], HIGH);
      }
      else
      {
        digitalWrite(modeLEDs[i], LOW);
      }
   }
}


// calls the required function based on active mode
void executeMode(uint8_t mode) {

  // for start index in color pallete mode
  // made static so that only initializes to 1 on the first call
  static uint16_t startIndex = 1;

  switch (mode) 
  {
    case 0:
      staticColourSet();
      break;
    case 1:
      startIndex++; /* motion speed */
      lightShow(startIndex);
      break;
    case 2:
      musicVisualizer();
      break;
    default:
      staticColourSet();
  }
}


// interrupt function to run on button press
void changeMode() {
  // reason for this condition is to correct hardware issue of button double triggering... Occurs due to noisy signal or mechanical fault
  // this condition disables the interrupt for 250ms after trigger thereby ignoring the double/triple trigger events
  if (millis() - lastInterrupt > 250)
  {
    state ++;  
    lastInterrupt = millis();
  }
}


// returns mapped inputs from input knobs in form of structure 'userSettings'
userSettings mapInputs (int range1, int range2, int range3) {
  userSettings mapped;

  // read and map the values
  mapped.inputA = map(analogRead(potReadPin[0]), 0, 1023, 0, range1);
  mapped.inputB = map(analogRead(potReadPin[1]), 0, 1023, 0, range2);
  mapped.inputC = map(analogRead(potReadPin[2]), 0, 1023, 0, range3);

  return mapped;
}


// MODE:0 - takes user input and maps to HSV colours
void staticColourSet() {
  // create variable of structure 'userSettings' and fill with mapped values
  userSettings colours = mapInputs(255, 255, 255);

  for (int i = 0; i < NUM_LEDS; i++)
  {
      leds[i] = CHSV(colours.inputA, colours.inputB, colours.inputC);
  }
}


// MODE:1 - goes through patterns of changing LEDs from the loaded palette
void lightShow(uint16_t startIndex) {

  userSettings pattern = mapInputs(6, 255, 255);

  // first knob is adjusted to choose the colour palette
  int paletteSet = pattern.inputA;

  // second knob is manipulate motion speed
  int speedControl = pattern.inputB;
  
  // third knob assigns brightness
  uint8_t brightness = pattern.inputC;

  switch (paletteSet)
  {
    case 0:
      currentPalette = RainbowColors_p;
      break;
    case 1:
      currentPalette = OceanColors_p;
      break;
    case 2:
      currentPalette = ForestColors_p;
      break;
    case 3:
      currentPalette = TorontoRaptors_p;
      break;
    case 4:
      currentPalette = Lakers_p;
     break;
    case 5 ... 6:
      currentPalette = India_p;
     break;
    default:
      currentPalette = RainbowColors_p;
  }
  
  // write to LEDS as required
  for( int i = 0; i < NUM_LEDS; i++) 
  {
      leds[i] = ColorFromPalette(currentPalette, startIndex, brightness, currentBlending);
      startIndex += speedControl;
  }
}


// MODE:2 - real time music visualizer (can configure origin LED, sensitivity and rate of colour change)
void musicVisualizer() {
  userSettings musicSettings = mapInputs(NUM_LEDS, 1023, 500);

  // first knob is to select point source LED
  int origin = musicSettings.inputA;

  // second knob is sound threshold to illuminate LEDs
  int threshold = musicSettings.inputB;

  // third knob decides time interval between colour increments
  int colourInterval = musicSettings.inputC;

  // initialize the colour which will be swept through
  static uint8_t colour = 1;

  // initialize time of last colour change
  static unsigned long lastClrChange;

  // increment the colour every specified amount of time
  if (millis() - lastClrChange > colourInterval)
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
}


// colour pixels at the chosen source on LED strip for music visualizer
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


// function takes an origin point LED and propagates the wave in both directions away from source for music visualizer
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
