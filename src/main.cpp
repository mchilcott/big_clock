#include <Arduino.h>

#include "OLED.h"
#include "Displays.h"
#include "HAL.h"
#include "ButtonMgr.h"

#include "ClockFace.h"
#include "FastLED.h"


// Define the array of leds
CRGB leds[kNumLEDs];


OLED display;

WindowManager mgr (&display);

Clock clk;
ClockTimer tmr;
CountUp stpw;

ListMenu<8> main_menu;



// Some small functions to pass in as pointers to the managers
void up ()
{ mgr.up_evt (); Serial.println("Up");}

void dn ()
{ mgr.down_evt (); Serial.println("Down");}

void entr ()
{ mgr.enter_evt (); Serial.println("Enter");}

void bk ()
{ mgr.back_evt (); Serial.println("Back");}

ButtonMgr btn_up (BTN_UP, &up, true);
ButtonMgr btn_dn (BTN_DOWN, &dn, true);
PressHoldMgr btn_opt (BTN_OPT, &entr, &bk);


void setup ()
{
  display.init();
  Serial.begin(9600);

  // Button Stuff
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_OPT, INPUT);
  digitalWrite(BTN_UP, HIGH);
  digitalWrite(BTN_DOWN, HIGH);
  digitalWrite(BTN_OPT, HIGH);

  btn_up.init();
  btn_dn.init();
  btn_opt.init();
  
  // Start up LEDs
  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, kNumLEDs);
  
  main_menu.add(&clk, "Clock");
  main_menu.add(&tmr, "Timer");
  main_menu.add(&stpw, "Stopwatch");
  mgr.load(&main_menu);
  
}

void loop ()
{
  mgr.run();
  btn_up.check_button();
  btn_dn.check_button();
  btn_opt.check_button();
  
}
