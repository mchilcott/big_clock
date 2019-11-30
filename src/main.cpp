#include <Arduino.h>

#include "OLED.h"
#include "Displays.h"
#include "HAL.h"
#include "ButtonMgr.h"

#include "ClockFace.h"
#include "FastLED.h"
#include "RTClib.h"

RTC_DS1307 rtc;


// Define the array of leds
CRGB leds[kNumLEDs];


OLED display;

WindowManager mgr (&display);

RTCClock clk;
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
  
  if (! rtc.begin()) {
    display.set_point(0,0);
    display.write("Couldn't find RTC");
    while (1);
  }
  rtc.writeSqwPinMode(DS1307_SquareWave1HZ);
  
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

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
