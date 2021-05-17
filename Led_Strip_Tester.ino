#include <JC_Button.h>
#include <FastLED.h>


enum State {Quantity, Brightness, Mode};


// Button pins
const uint8_t PIN_BTN_LEFT   = A5;
const uint8_t PIN_BTN_RIGHT  = A4;
const uint8_t PIN_BTN_STATE  = A3;

// Display pins
const uint8_t PIN_DIS_SER   = 6;
const uint8_t PIN_DIS_OE    = 7;
const uint8_t PIN_DIS_RCLK  = 8;
const uint8_t PIN_DIS_SRCLK = 9;
const uint8_t PIN_DIS_SRCLR = 10;

// State variables
uint8_t state_button_state = State::Quantity;
const unsigned long clear_hold_time = 1000; // time in milliseconds for clear button hold to activate
unsigned long clear_hold_start = 0;
bool justCleared = false;

// FastLED variables
const uint8_t MAX_LEDS  = 64;
const uint8_t PIN_DATA  = 3;
#define CHIPSET         = WS2812B; // template variable cannot easily be stored as a non-constant variable. Will look into for switching


typedef void LedFunc(unsigned long);

// Mode prototypes
void Mode_1(unsigned long);
void Mode_2(unsigned long);
void Mode_3(unsigned long);

// Variables
uint8_t GlobalBrightness = 64;
uint8_t GlobalQuantity   = 4;
uint8_t GlobalMode       = 0;

LedFunc* ledModes[] = {
  Mode_1,
  Mode_2,
  Mode_3
};
const uint8_t MAX_MODES = sizeof(ledModes)/sizeof(LedFunc*);


// Buttons
Button button_left(PIN_BTN_LEFT);
Button button_right(PIN_BTN_RIGHT);
Button button_state(PIN_BTN_STATE);

CRGB leds[MAX_LEDS];

void setup()
{
  Serial.begin(9600);
  Serial.println("Ready!");
  Serial.println(MAX_MODES);
  
  button_left.begin();
  button_right.begin();
  button_state.begin();

  FastLED.addLeds<WS2812B, PIN_DATA, GRB>(leds, MAX_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(GlobalBrightness);

  FastLED.clear(true);
}


void updateButtons(unsigned long time)
{
  button_left.read();
  button_right.read();
  button_state.read();
  
  if (button_left.wasPressed()) {
    left();
  }
  
  if (button_right.wasPressed()) {
    right();
  }

  if (button_state.wasPressed()) {
    switch_state();
  }
}

void updateLeds(unsigned long time)
{
  ledModes[GlobalMode](time);
}

/* Mode procedures */
void Mode_1(unsigned long time)
{
   fill_rainbow(leds, GlobalQuantity, (time/10)%255, 255/GlobalQuantity);
   FastLED.show();
}

void Mode_2(unsigned long time)
{
  static CHSV color(0, 255, 255);
  color.hue = (color.hue+1) % 255;
  fill_solid(leds, GlobalQuantity, color);
}

void Mode_3(unsigned long time)
{
}

/* Button procedures*/
void left()
{ 
  switch(state_button_state) {
    case State::Quantity:
      GlobalQuantity = max(1, GlobalQuantity - 1);
      clearLeds(GlobalQuantity);
      break;
    case State::Brightness:
      GlobalBrightness = max(0, GlobalBrightness-8);
      setBrightness(GlobalBrightness);
      break;
    case State::Mode:
      GlobalMode = max(0, GlobalMode - 1);
      break;
  }

  Serial.print(GlobalQuantity);
  Serial.print(" - ");
  Serial.print(GlobalBrightness);
  Serial.print(" - ");
  Serial.print(GlobalMode);
  Serial.println();
}

void right()
{
  switch(state_button_state) {
    case State::Quantity:
      GlobalQuantity = min(MAX_LEDS, GlobalQuantity + 1);
      clearLeds(GlobalQuantity);
      break;
    case State::Brightness:
      GlobalBrightness = min(255, GlobalBrightness + 8);
      setBrightness(GlobalBrightness);
      break;
    case State::Mode:
      GlobalMode = min(MAX_MODES-1, GlobalMode + 1);
      break;
  }

  Serial.print(GlobalQuantity);
  Serial.print(" - ");
  Serial.print(GlobalBrightness);
  Serial.print(" - ");
  Serial.print(GlobalMode);
  Serial.println();
}

void clearLeds(uint8_t fromIndex)
{
  for (uint8_t i = fromIndex; i < MAX_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

void setBrightness(uint8_t brightness)
{
  FastLED.setBrightness(brightness);
}

void switch_state()
{
  switch (state_button_state) {
    case State::Quantity:
      state_button_state = State::Brightness;
      Serial.println("Switch state to State::Brightness");
      break;
    case State::Brightness:
      state_button_state = State::Mode;
      Serial.println("Switch state to State::Mode");
      break;
    case State::Mode:
      state_button_state = State::Quantity;
      Serial.println("Switch state to State::Quantity");
      break;
  }
}


void loop()
{
  unsigned long time = millis();

  updateButtons(time);
  updateLeds(time);

  FastLED.delay(16);
}
