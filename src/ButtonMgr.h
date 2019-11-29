#pragma once

#include <Arduino.h>

// Minimum button press time to check for change
#define DEBOUNCE_TIME 15

// Time till hold event emitted for PressHoldMgr
#define HOLD_TIME 600

// Button Logic levels
#define PRESSED LOW
#define RELEASED HIGH


class ButtonMgr
{
public:

 ButtonMgr(int btn_pin, void (* callback) (), bool multi_press = false):
  mPin(btn_pin),
    mMulti (multi_press)
    {
      mCallback = callback;
    }
  
  void init () {
    mLastState = digitalRead(mPin);  
  }

  virtual void check_button()
  {
    bool new_state = digitalRead(mPin);
    if(new_state == mLastState && mWaitTime == 0)
      // We are not waiting, and there has been no change
      return;

    if(new_state == mLastState && new_state == PRESSED && (signed long)(millis() - mWaitTime) > DEBOUNCE_TIME)
      {
	// We have waited
	mCallback();
	mNumCalls ++;

	if(mMulti)
	  {
	    // Decrease wait each time (hold accelerates presses)
	    int offset =  300 - (mNumCalls << 4);
	    if (offset < 0) offset = 0;
	    
	    mWaitTime = millis() + offset; // wait for press in future
	  }
	else
	  mWaitTime = 0; // Signal no longer waiting

	return;
      }

    if(new_state != mLastState)
      {
	mLastState = new_state;
	
	if(new_state == PRESSED)
	  mWaitTime = millis();
	else
	  mWaitTime  = 0;
	
	mNumCalls = 0;
      }
  }


protected:
  // Am I waiting for debounce test
  unsigned long mWaitTime;
  // Last known state of the button
  bool mLastState;
  
  // Callback function on button press
  void (* mCallback) ();

  // IO pin of button
  int mPin;

  // Should we make multiple calls if the button is held
  bool mMulti;

  // Number of callbacks for this button press
  unsigned int mNumCalls;
};



class PressHoldMgr: public ButtonMgr
{
public:

  PressHoldMgr (int pin_num, void (* callback)(), void (* hold_callback)()):
    ButtonMgr(pin_num, callback, false),
    mCallOnRelease(false)
  {mHoldCallback = hold_callback;}
 
  virtual void check_button ()
    {
    bool new_state = digitalRead(mPin);
    
    if(new_state == RELEASED && mLastState == RELEASED)
      // Button not pressed
      return;
    
    if(new_state == mLastState && new_state == PRESSED && millis() - mWaitTime > HOLD_TIME && mNumCalls == 0)
      {
	// We have waited so long that this is a hold event, and this hasn't been called before
	mHoldCallback();
	mNumCalls ++;

        mWaitTime = 0; // Signal no longer waiting
        mCallOnRelease = false;
	return;
      }
    else if (new_state == mLastState && new_state == PRESSED && millis() - mWaitTime > DEBOUNCE_TIME && mNumCalls == 0)
      {
        // Waited for debounce, and we haven't already called the hold function
        mCallOnRelease = true;
        return;
      }

    // Handle changes
    if(new_state != mLastState)
      {
	mLastState = new_state;
	
	if(new_state == PRESSED)
	  mWaitTime = millis();
	else
          {
	  mWaitTime = 0;
          if(mCallOnRelease)
            mCallback();
          mCallOnRelease = false;
          }
	
	mNumCalls = 0;
      }
  }

private:
  // Callback for hold
  void (* mHoldCallback) ();

  // Will we make a call on button release? (i.e. have we not held too long)
  bool mCallOnRelease;

};
