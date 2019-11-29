#pragma once

#include "OLED.h"
#include "HAL.h"
#include "FastLED.h"
#include "ClockFace.h"

class WindowManager;

class Window
{

public:
  // Button actions
  virtual void up ();
  virtual void down ();
  virtual void back ();
  virtual void enter ();

  // Draw the window
  virtual void draw(OLED * display);

  // private: But not really private
  WindowManager * mgr; // Manager for context
  Window * parent = nullptr;     // Parent window (for back)
};

class WindowManager
{

public:
  // Make a new WindowManager on an OLED Display
  WindowManager(OLED * display)
  {
    this->display = display;
  }
  
  // Run the window manager
  // i.e draw the current window, and do some button management
  void run()
  {
    current->draw(display);
    if (Serial.available() > 0)
      {
        // read the incoming byte:
        char incomingByte = Serial.read();
        switch (incomingByte)
          {
          case 'n':
            current->down();
            break;
          case 'p':
            current->up();
            break;
          case 'b':
            current->back();
            break;
          case 'e':
            current->enter();
            break;
          }
      }
  }

  void down_evt()
  {current->down();}

  void up_evt()
  {current->up();}
  
  void enter_evt()
  {current->enter();}
  
  void back_evt()
  {current->back();}

  // Change the displayed window
  void load(Window * wind){
    display->clear();
    delay(5);
    current = wind;
    wind->mgr = this;
  }

  void clean()
  {
    display->clear();
  }
  
protected:
  Window * current;
  OLED * display;
};


// This menu offers a list of windows.
template <unsigned char T>
class ListMenu : public Window
{

public:
  void add (Window * wind, const char * label)
  {
    // Set up back pointers. This gives the child access to the
    // manager, and tells it that we are the parent pane in the tree
    // hiriachy
    if(wind)
      {
        wind->parent = this;
        wind->mgr = mgr;
      }
    // Only add it if we have space (in the template)
    if(count < T)
      {
        mLabels[count] = label;
        mWindows[count] = wind;
        count ++;
      }
  }

  virtual void draw (OLED *disp)
  {
    unsigned char start, num;

    // Select which part of the list to display
    if (count <= DISP_HEIGHT)
      {
        // Doesn't fill window, so display everything
        start = 0;
        num = count;
      }
    else if (ind == 0)
      {
        // Index is the start. Display the first few items
        start = 0;
        num = DISP_HEIGHT;
      }
    else if (count - ind < DISP_HEIGHT - 1)
      {
        // There are less items after the index than required to fill
        // the display if the current index is displayed second.
        start = count - DISP_HEIGHT;
        num = DISP_HEIGHT;
      }
    else
      {
        // The current index will be displayed second
        start = ind - 1;
        num = DISP_HEIGHT;
      }

    // Draw a label on each line
    for (unsigned char i = 0; i < num; ++i)
      {
        // Translate from line number to index in labels
        unsigned char it = i + start;
        
        disp->set_point (i, 0);

        // If the current line is the current index, mark it
        if(it == ind)
          disp->write("-> ");

        disp->write(mLabels[it]);
        disp->write("   "); // Clear any artifacts from the "-> "
      }
  }

  // Go to next menu item
  virtual void down ()
  {
    ind = (ind + 1) % count;
  }

  // Go to prev menu item
  virtual void up ()
  {
    if(ind == 0)
      ind = count - 1;
    else
      ind--;
  }

  // Run the selected window
  virtual void enter ()
  {
    if(mWindows[ind])
      {
        mgr->load(mWindows[ind]);
      }     
  }

  // Go to parent window
  virtual void back ()
  {
    if(parent)
      {
        mgr->load(parent);
      }
  }

private:

  // List of labels for menu
  const char * mLabels [T];
  // List of windows to go to
  Window * mWindows [T];

  // Number of windows added
  unsigned char count = 0;

  // Currently selected menu item
  unsigned char ind = 0;
};



class TextWindow: public Window
{
public:
  TextWindow (const char * str)
  {
    mText = str;
    mStart = 0;
    
    mLength = 0;
    while(*str != '\0')
      {
        str++;
        mLength ++;
      }
  }

  virtual void up ()
  {
    if(mStart < DISP_WIDTH)
      mStart = 0;
    else
      mStart -= DISP_WIDTH;
    mgr->clean();
    Serial.print((int)mLength, DEC);
  }
  
  virtual void down ()
  {
    if (mLength - mStart  <  DISP_HEIGHT * DISP_WIDTH)
      return;
    else
      mStart += DISP_WIDTH;
    mgr->clean();
  }
  
  virtual void back ()
  {
    mgr->load(parent);
  }
  
  virtual void enter ()
  {}

  // Draw the window
  virtual void draw(OLED * display){
    const char * it = mText + mStart;
    for(unsigned char i = 0; i < DISP_HEIGHT; ++i)
      {
        display->set_point(i, 0);
        for(unsigned char j = 0; j < DISP_WIDTH; ++j)
          {
            if(*it == '\0')
              return;
            display->data(*it++);
          }
      }
  }

private:
  const char * mText;
  unsigned int mLength;
  unsigned int mStart;
};


class Clock: public Window
{
public:
  Clock ():
    mEditState(k_none),
    mLast(millis()),
    year(2019),
    month(11),
    day(23),
    hr(11),
    minu(15),
    sec(00)
  {}


  virtual void up ()
  {
    switch(mEditState)
      {
      case k_day:
        day ++; break;

      case k_month:
        month ++; break;

      case k_year:
        year ++; break;

      case k_hr:
        hr ++; break;

      case k_min:
        minu ++; break;

      case k_sec:
        sec ++; break;                   
                  
      default:
      case k_none:
        break;
      }
    update_forward();
  }

  
  virtual void down ()
  {
    switch(mEditState)
      {
      case k_day:
        day --; break;

      case k_month:
        month --; break;

      case k_year:
        year --; break;

      case k_hr:
        hr --; break;

      case k_min:
        minu --; break;

      case k_sec:
        sec --; break;                   
                  
      default:
      case k_none:
        break;
      }
    update_backward();
  }

  
  virtual void back ()
  {
    if(parent)
      mgr->load(parent);
  }

  
  virtual void enter ()
  {
    switch(mEditState)
      {
      case k_day:
        mEditState = k_month; break;

      case k_month:
        mEditState = k_year; break;

      case k_year:
        mEditState = k_hr; break;

      case k_hr:
        mEditState = k_min; break;

      case k_min:
        mEditState = k_sec; break;

      case k_sec:
        mEditState = k_none; break;      
                  
      default:
      case k_none:
        mEditState = k_day; break;
      }
    mNeedsClear = true;
  }

  // Draw the window
  virtual void draw(OLED * disp)
  {
    if(mNeedsClear)
    {
      disp->clear();
      mNeedsClear = false;
    }
    
    // Draw time
    char buf [20];
    sprintf(buf, "%2d/%02d/%4d  ", day, month, year);
    disp->set_point(1,3);
    disp->write(buf);

    sprintf(buf, "%2d:%02d:%02d  ", hr, minu, sec);
    disp->set_point(2,6);
    disp->write(buf);
    
    CRGB Colour = {0x0F,0x1F,0};
    
    sprintf(buf, "%02d%02d%02d", hr, minu, sec);
    
    set_segment_display(leds, kDigitStart[0], get_rep(buf[0]), Colour);
    set_segment_display(leds, kDigitStart[1], get_rep(buf[1]), Colour);
    set_segment_display(leds, kDigitStart[2], get_rep(buf[2]), Colour);
    set_segment_display(leds, kDigitStart[3], get_rep(buf[3]), Colour);
    set_segment_display(leds, kDigitStart[4], get_rep(buf[4]), Colour);
    set_segment_display(leds, kDigitStart[5], get_rep(buf[5]), Colour);
    
    if(sec%2){
        set_colon(leds,kDigitStart[1],{0x0F,0x1F,0});
        set_colon(leds,kDigitStart[3],{0x0F,0x1F,0});
    } else {
        set_colon(leds,kDigitStart[1],{0x00,0x0F,0x1F});
        set_colon(leds,kDigitStart[3],{0x00,0x0F,0x1F});
    }
    
    FastLED.show();

    // Draw highlight
    switch(mEditState)
      {
      case k_day:
        disp->set_point(0,3); disp->write("\x1B\x1B"); break;
        
      case k_month:
        disp->set_point(0,6); disp->write("\x1B\x1B"); break;
        
      case k_year:
        disp->set_point(0,9); disp->write("\x1B\x1B\x1B\x1B"); break;
        
      case k_hr:
        disp->set_point(3,6); disp->write("\x1A\x1A"); break;
        
      case k_min:
        disp->set_point(3,9); disp->write("\x1A\x1A"); break;

      case k_sec:
        disp->set_point(3,12); disp->write("\x1A\x1A"); break;
        
      default:
      case k_none:
        break;
      }
    

    while(millis() - mLast > 1000)
      {
        mLast += 1000;
        sec ++;
        update_forward();   
      }
  }
  
  
private:
  typedef enum {k_none, k_day, k_month, k_year, k_hr, k_min, k_sec} edit_t;

  edit_t mEditState;
  unsigned char day, month, hr, minu, sec;
  unsigned int year;

  unsigned long mLast;

  bool mNeedsClear = false;

  // Update moving forward in time
  void update_forward ()
  {
    // Update times
    if(sec > 59)
      {sec = 0; minu ++;}
    if(minu > 59)
      {minu = 0; hr ++;}
    if(hr > 23)
      {hr = 0; day ++;}
    if(day > len_month())
      {day = 1; month++;}
    if(month > 12)
      {month = 1; year++;}
  }

  // Update moving backwards in time
  void update_backward ()
  {
    bool day_change = false;
    bool month_change = false;
    // Reverse (0xFF == -1 which is greater than 59)
    if(sec > 59)
      {sec = 59; minu --;}
    if(minu > 59)
      {minu = 59; hr --;}
    if(hr > 23)
      {hr = 23; day --;}

    // Day rollover
    if(day == 0 || day > 31)
      {month--;}

    // month rollover
    if(month == 0 || month > 12)
      {month = 12; year--;}
      
    if(day == 0 || day > len_month())
      {day = len_month();}
  }

  unsigned char len_month()
  {
    char lens [] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if(month != 2)
      return lens[month - 1];

    if (year % 4 == 0 && (year % 100 != 0 || year & 400 == 0))
      return 29;
    else
      return 28;
    
  }
  
};


class ClockTimer: public Window
{
public:
  ClockTimer ():
    mEditState(k_none),
    mLast(millis()),
    hr(0),
    minu(15),
    sec(00)
  {}


  virtual void up ()
  {
    switch(mEditState)
      {
      case k_hr:
        hr ++; break;

      case k_min:
        minu ++; break;

      case k_sec:
        sec ++; break;                   
                  
      default:
      case k_none:
        break;
      }
    update_forward();
  }

  
  virtual void down ()
  {
    switch(mEditState)
      {
      case k_hr:
        hr --; break;

      case k_min:
        minu --; break;

      case k_sec:
        sec --; break;                   
                  
      default:
      case k_none:
        break;
      }
    update_backward();
  }

  
  virtual void back ()
  {
    if(parent)
      mgr->load(parent);
  }

  
  virtual void enter ()
  {
    switch(mEditState)
      {
      case k_hr:
        mEditState = k_min; break;

      case k_min:
        mEditState = k_sec; break;

      case k_sec:
        mEditState = k_run; break;

      case k_run:
        mEditState = k_none; break;
                  
      default:
      case k_none:
        mEditState = k_hr; break;
      }
    mNeedsClear = true;
  }

  // Draw the window
  virtual void draw(OLED * disp)
  {
    if(mNeedsClear)
    {
      disp->clear();
      mNeedsClear = false;
    }
    
    // Draw time
    char buf [20];

    sprintf(buf, "%2d:%02d:%02d  ", hr, minu, sec);
    disp->set_point(2,6);
    disp->write(buf);
    
    CRGB Colour = {0x0F,0x1F,0};

    if (hr > 0 || minu > 3)
      {
        Colour.r = 0x00;
        Colour.g = 0x20;
        Colour.b = 0x00;
      }

    if (hr == 0 && minu < 3)
      {
        Colour.r = 0x1F;
        Colour.g = 0x1F;
        Colour.b = 0x00;
      }

    if (hr == 0 && minu < 1)
      {
        Colour.r = 0x40;
        Colour.g = 0x00;
        Colour.b = 0x00;
      }
    
    sprintf(buf, "%02d%02d%02d", hr, minu, sec);

    set_colon(leds,kDigitStart[0],{0,0,0});
    set_colon(leds,kDigitStart[1],{0,0,0});
    set_colon(leds,kDigitStart[2],{0,0,0});
    set_colon(leds,kDigitStart[3],{0,0,0});
    set_colon(leds,kDigitStart[4],{0,0,0});
    set_colon(leds,kDigitStart[5],{0,0,0});
    
    if (hr > 0)
      {
        // Use all six characters
        set_segment_display(leds, kDigitStart[0], get_rep(buf[0]), Colour);
        set_segment_display(leds, kDigitStart[1], get_rep(buf[1]), Colour);
        set_segment_display(leds, kDigitStart[2], get_rep(buf[2]), Colour);
        set_segment_display(leds, kDigitStart[3], get_rep(buf[3]), Colour);
        set_segment_display(leds, kDigitStart[4], get_rep(buf[4]), Colour);
        set_segment_display(leds, kDigitStart[5], get_rep(buf[5]), Colour);
    
        if(sec%2){
          set_colon(leds,kDigitStart[1],Colour);
          set_colon(leds,kDigitStart[3],Colour);
        } else {
          set_colon(leds,kDigitStart[1],{0,0,0});
          set_colon(leds,kDigitStart[3],{0,0,0});
        }
      }
    else
      {
        // Use 4 middle characters
        set_segment_display(leds, kDigitStart[1], get_rep(buf[2]), Colour);
        set_segment_display(leds, kDigitStart[2], get_rep(buf[3]), Colour);
        set_segment_display(leds, kDigitStart[3], get_rep(buf[4]), Colour);
        set_segment_display(leds, kDigitStart[4], get_rep(buf[5]), Colour);

        set_segment_display(leds, kDigitStart[0], 0, Colour);
        set_segment_display(leds, kDigitStart[5], 0, Colour);
        
        if(sec%2){
          set_colon(leds,kDigitStart[2],Colour);
        } else {
          set_colon(leds,kDigitStart[2],{0,0,0});

        }
      }
    
    FastLED.show();

    // Draw highlight
    switch(mEditState)
      {
        
      case k_hr:
        disp->set_point(3,6); disp->write("\x1A\x1A"); break;
        
      case k_min:
        disp->set_point(3,9); disp->write("\x1A\x1A"); break;

      case k_sec:
        disp->set_point(3,12); disp->write("\x1A\x1A"); break;
        
      default:
        break;
      }
    

    switch(mEditState)
      {

      case k_run:
        while(millis() - mLast > 1000)
          {
            mLast += 1000;
            sec --;
            update_backward();

            if (hr == 0 && minu == 0 && sec == 0)
              {
                mEditState = k_done;
                break;
              }
          }
        break;

      default:
        mLast = millis();
      }

  }
  
  
private:
  typedef enum {k_none, k_hr, k_min, k_sec, k_run, k_done} state_t;

  state_t mEditState;
  unsigned char hr, minu, sec;

  unsigned long mLast;

  bool mNeedsClear = false;

  // Update moving forward in time
  void update_forward ()
  {
    // Update times
    if(sec > 59)
      {sec = 0; minu ++;}
    if(minu > 59)
      {minu = 0; hr ++;}

    if (hr > 99)
      {hr = 0;}
  }

  // Update moving backwards in time
  void update_backward ()
  {
    if(sec > 59)
      {sec = 59; minu --;}
    if(minu > 59)
      {minu = 59; hr --;}
    if (hr > 99)
      {hr = 99;}
  }


  
};

class CountUp: public Window
{
public:
  CountUp ():
    mLast(millis()),
    hr(0),
    minu(0),
    sec(0),
    mRunning(false)
      {}


  virtual void up ()
  {

  }

  
  virtual void down ()
  {
    hr = minu = sec = 0;
  }

  
  virtual void back ()
  {
    if(parent)
      mgr->load(parent);
  }

  
  virtual void enter ()
  {
    mRunning = !mRunning;
  }

  // Draw the window
  virtual void draw(OLED * disp)
  {
    if(mNeedsClear)
    {
      disp->clear();
      mNeedsClear = false;
    }
    
    // Draw time
    char buf [20];

    sprintf(buf, "%2d:%02d:%02d  ", hr, minu, sec);
    disp->set_point(2,6);
    disp->write(buf);
    
    CRGB Colour = {0x0F,0x1F,0};

    
    sprintf(buf, "%02d%02d%02d", hr, minu, sec);

    set_colon(leds,kDigitStart[0],{0,0,0});
    set_colon(leds,kDigitStart[1],{0,0,0});
    set_colon(leds,kDigitStart[2],{0,0,0});
    set_colon(leds,kDigitStart[3],{0,0,0});
    set_colon(leds,kDigitStart[4],{0,0,0});
    set_colon(leds,kDigitStart[5],{0,0,0});
    
    if (hr > 0)
      {
        // Use all six characters
        set_segment_display(leds, kDigitStart[0], get_rep(buf[0]), Colour);
        set_segment_display(leds, kDigitStart[1], get_rep(buf[1]), Colour);
        set_segment_display(leds, kDigitStart[2], get_rep(buf[2]), Colour);
        set_segment_display(leds, kDigitStart[3], get_rep(buf[3]), Colour);
        set_segment_display(leds, kDigitStart[4], get_rep(buf[4]), Colour);
        set_segment_display(leds, kDigitStart[5], get_rep(buf[5]), Colour);
    
        if(sec%2){
          set_colon(leds,kDigitStart[1],Colour);
          set_colon(leds,kDigitStart[3],Colour);
        } else {
          set_colon(leds,kDigitStart[1],{0,0,0});
          set_colon(leds,kDigitStart[3],{0,0,0});
        }
      }
    else
      {
        // Use 4 middle characters
        set_segment_display(leds, kDigitStart[1], get_rep(buf[2]), Colour);
        set_segment_display(leds, kDigitStart[2], get_rep(buf[3]), Colour);
        set_segment_display(leds, kDigitStart[3], get_rep(buf[4]), Colour);
        set_segment_display(leds, kDigitStart[4], get_rep(buf[5]), Colour);

        set_segment_display(leds, kDigitStart[0], 0, Colour);
        set_segment_display(leds, kDigitStart[5], 0, Colour);
        
        if(sec%2){
          set_colon(leds,kDigitStart[2],Colour);
        } else {
          set_colon(leds,kDigitStart[2],{0,0,0});

        }
      }
    
    FastLED.show();

    

    if (mRunning)
      {
        while(millis() - mLast > 1000)
          {
            mLast += 1000;
            sec ++;
            update_forward();   
          }
      }
    else
      {
        mLast = millis();
      }
    

  }
  
  
private:

  bool mRunning;
  unsigned char hr, minu, sec;

  unsigned long mLast;

  bool mNeedsClear = false;

  // Update moving forward in time
  void update_forward ()
  {
    // Update times
    if(sec > 59)
      {sec = 0; minu ++;}
    if(minu > 59)
      {minu = 0; hr ++;}
  }
  
};

/*

// This could do with tidying up
template <typename T> class Modifier : public Window
{

public:
  Modifier<T> (const char *name, const char *format, T *ref, T min, T max):
  mName(name), mValue(ref), mMin(min), mMax(max), mFormat(format), mIncrement(1)
  {}
    
  // Button actions
  virtual void up ()
  {
    Serial.println("inc");
    (*mValue) += mIncrement;
    if (*mValue > mMax) *mValue = mMax;
  }
  
  virtual void down ()
  {
    Serial.println("dec");
    (*mValue) -= mIncrement;
    if (*mValue < mMin) *mValue = mMin;
  }

  virtual void back ()
  {
    if(parent)
      mgr->load(parent);
  }

  virtual void enter ()
  {
  }

  // Draw the window
  virtual void draw(OLED * disp)
  {
    char buffer [20];

    disp->set_point(1,0);
    disp->write(mName);

     PString str(buffer, sizeof(buffer));

    str.print("Val: ");
    str.print(*mValue);
    disp->set_point(2,0);
    disp->write(str);

    str.begin();
    str.print("Inc: ");
    str.print(mIncrement);
    disp->set_point(3,0);
    disp->write(str);
  }


 private:
  T *mValue;
  T mMin;
  T mMax;
  T mIncrement;
  const char *mName;
  const char *mFormat;
};
*/
