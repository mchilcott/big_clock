#pragma once

#include "Wire.h"
#include "Arduino.h"

class OLED {

 public:
  OLED (int i2c_address = 0x3C){
    // 0x3C or 0x78 are usual addresses
    slave2w = i2c_address;
  }
  
  // Start up the display on I2C
  void init ();

  // Write character to display
  void data (unsigned char d);

  // Send command
  void command (unsigned char c);

  // Clear display - Note, I think it is advantageous to pause after this command before writing anything else
  void clear () 
  { command (0x01); delay(10); }

  // Set the character insertion address at the given line and character
  void set_point (unsigned char line, unsigned char pos);

  // Write the given string (Writes a max of 20 characters)
  void write (const char * str);
  
 private:
  // Send a raw packet
  void send_packet();

  // I2C address
  unsigned char slave2w;
  unsigned char tx_packet[2] = {0x00, 0x00};
  
};
