#include <JC_Button.h>
#include <RotaryEncoder.h>
#include <FastLED.h>


enum State {Quantity, Brightness, Mode};


// Button pins
const uint8_t PIN_ROT_LEFT   = A3;
const uint8_t PIN_ROT_RIGHT  = A2;
const uint8_t PIN_BTN_STATE  = A1;

// Display pins
const uint8_t PIN_DIS_SER   = 5;
const uint8_t PIN_DIS_OE    = 6;
const uint8_t PIN_DIS_RCLK  = 7;
const uint8_t PIN_DIS_SRCLK = 8;
const uint8_t PIN_DIS_SRCLR = 9;
const uint8_t PIN_DIS_DIGIT_TEN = 3;
const uint8_t PIN_DIS_DIGIT_ONE = 2;

// State variables
uint8_t state_button_state = State::Quantity;
const unsigned long clear_hold_time = 1000; // time in milliseconds for clear button hold to activate
unsigned long clear_hold_start = 0;
bool justCleared = false;

// FastLED variables
const uint8_t MAX_LEDS  = 100;
const uint8_t PIN_DATA  = 4;
#define CHIPSET         = WS2812B; // template variable cannot easily be stored as a non-constant variable. Will look into for switching


typedef void LedFunc(double, double);

// Mode prototypes
void Mode_1(double, double);
void Mode_2(double, double);
void Mode_3(double, double);

// Variables
uint8_t GlobalBrightness = 25;
uint8_t GlobalQuantity   = 5;
uint8_t GlobalMode       = 0;

LedFunc* ledModes[] = {
  Mode_1,
  Mode_2,
  Mode_3
};
const uint8_t MAX_MODES = sizeof(ledModes)/sizeof(LedFunc*);


// 7 Segment display
const byte display_patterns[16] =
{
  B00111111,  // 0
  B00000110,  // 1
  B01011011,  // 2
  B01001111,  // 3
  B01100110,  // 4
  B01101101,  // 5
  B01111101,  // 6
  B00000111,  // 7
  B01111111,  // 8
  B01101111,  // 9
  B01110111,  // A
  B01111100,  // b
  B00111001,  // C
  B01011110,  // d
  B01111001,  // E
  B01110001   // F
};

//const byte display_progress[] = {
//  B00000000,
//};

const uint8_t FPS = 60;
const double frametime = 1.0/FPS;

uint8_t display_data[2] = {0, 0};
uint8_t display_value = 0;


// Buttons
RotaryEncoder encoder(PIN_ROT_LEFT, PIN_ROT_RIGHT, RotaryEncoder::LatchMode::TWO03);
Button button_state(PIN_BTN_STATE);

CRGB leds[MAX_LEDS];

void setup()
{
  pinMode(PIN_DIS_SER,       OUTPUT);
  pinMode(PIN_DIS_OE,        OUTPUT);
  pinMode(PIN_DIS_RCLK,      OUTPUT);
  pinMode(PIN_DIS_SRCLK,     OUTPUT);
  pinMode(PIN_DIS_SRCLR,     OUTPUT);
  pinMode(PIN_DIS_DIGIT_TEN, OUTPUT);
  pinMode(PIN_DIS_DIGIT_ONE, OUTPUT);

  pinMode(PIN_BTN_STATE, INPUT_PULLUP);

  digitalWrite(PIN_DIS_OE, HIGH);
  
  Serial.begin(9600);
  Serial.println("Ready!");
  Serial.println(MAX_MODES);
  
  button_state.begin();

  FastLED.addLeds<WS2812B, PIN_DATA, GRB>(leds, MAX_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  setBrightness(GlobalBrightness);

  FastLED.clear(true);

  setDisplay(GlobalQuantity);
}


void updateButtons(double time, double delta)
{
  encoder.tick();
  button_state.read();

  RotaryEncoder::Direction encDir = encoder.getDirection();
  
  if (encDir == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
    left();
  }
  
  if (encDir == RotaryEncoder::Direction::CLOCKWISE) {
    right();
  }

  if (button_state.wasPressed()) {
    switch_state();
  }
}

void updateLeds(double time, double delta)
{
  ledModes[GlobalMode](time, delta);
}

void updateDisplay(double time, double delta)
{
  static uint8_t index = 0;

  index = !index;

  digitalWrite(PIN_DIS_OE, LOW);
  digitalWrite(PIN_DIS_SRCLR, HIGH);
  
  digitalWrite(PIN_DIS_RCLK, LOW);
  shiftOut(PIN_DIS_SER, PIN_DIS_SRCLK, MSBFIRST, ~display_data[index]);
  digitalWrite(PIN_DIS_RCLK, HIGH);
  
  if (index == 1) {
    digitalWrite(PIN_DIS_DIGIT_ONE, HIGH);
    digitalWrite(PIN_DIS_DIGIT_TEN, LOW);
  } else {
    digitalWrite(PIN_DIS_DIGIT_ONE, LOW);
    digitalWrite(PIN_DIS_DIGIT_TEN, HIGH);
  }
}
/* Mode procedures */
void Mode_1(double time, double delta)
{
   fill_rainbow(leds, GlobalQuantity, int(time*100)%255, 255/GlobalQuantity);
   FastLED.show();
}

void Mode_2(double time, double delta)
{
  static CHSV color(0, 255, 255);
  static double hue = 0;
  hue = fmod(hue + 50.0f * delta, 256);
  color.hue = int(hue);
  fill_solid(leds, GlobalQuantity, color);
  FastLED.show();
}

void Mode_3(double time, double delta)
{
}

/* Button procedures*/
void left()
{ 
  switch(state_button_state) {
    case State::Quantity:
      GlobalQuantity = max(1, GlobalQuantity - 1);
      setDisplay(GlobalQuantity);
      clearLedsAfter(GlobalQuantity);
      break;
    case State::Brightness:
      GlobalBrightness = max(0, GlobalBrightness - 5);
      setDisplay(GlobalBrightness);
      setBrightness(GlobalBrightness);
      break;
    case State::Mode:
      GlobalMode = max(0, GlobalMode - 1);
      setDisplay(GlobalMode);
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
      setDisplay(GlobalQuantity);
      clearLedsAfter(GlobalQuantity);
      break;
    case State::Brightness:
      GlobalBrightness = min(99, GlobalBrightness + 5);
      setDisplay(GlobalBrightness);
      setBrightness(GlobalBrightness);
      break;
    case State::Mode:
      GlobalMode = min(MAX_MODES-1, GlobalMode + 1);
      setDisplay(GlobalMode);
      break;
  }

  Serial.print(GlobalQuantity);
  Serial.print(" - ");
  Serial.print(GlobalBrightness);
  Serial.print(" - ");
  Serial.print(GlobalMode);
  Serial.println();
}

void clearLedsAfter(uint8_t fromIndex)
{
  for (uint8_t i = fromIndex; i < MAX_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

void setBrightness(uint8_t brightness)
{
  brightness = ((brightness + 5 - 1) / 5) * 5; // round to nearest 5;
  GlobalBrightness = brightness;
  Serial.println(brightness);
  FastLED.setBrightness((brightness*255) / 100);
}

void switch_state()
{
  switch (state_button_state) {
    case State::Quantity:
      state_button_state = State::Brightness;
      setDisplay(GlobalBrightness);
      Serial.println("Switch state to State::Brightness");
      break;
    case State::Brightness:
      state_button_state = State::Mode;
      setDisplay(GlobalMode);
      Serial.println("Switch state to State::Mode");
      break;
    case State::Mode:
      state_button_state = State::Quantity;
      setDisplay(GlobalQuantity);
      Serial.println("Switch state to State::Quantity");
      break;
  }
}

void setDisplay(int value)
{
  uint8_t tens = value % 10;
  uint8_t ones = (int)(value % 100) / 10;

  display_data[1] = display_patterns[tens];
  display_data[0] = display_patterns[ones];
}

void loop()
{
  static double lastTime = 0.016;
  double time = (millis() / 1000.0f);
  double delta = time - lastTime; // I don't know if this is the proper way to do this, but it's working for now. TODO: Investigate

  updateButtons(time, delta);
  updateLeds(time, delta);
  updateDisplay(time, delta);

  lastTime = time;

  FastLED.delay(int(frametime - delta) * 1000);
}
