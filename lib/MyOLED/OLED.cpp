#include "OLED.h"

// Part some code / ideas from https://github.com/NewhavenDisplay/NHD_US2066.git

void OLED::command (unsigned char c)
{
  tx_packet[0] = 0x00;
  tx_packet[1] = c;
  send_packet();
}

void OLED::data (unsigned char d)
{

  tx_packet[0] = 0x40;
  tx_packet[1] = d;
  send_packet();
}

void OLED::send_packet ()
{
  unsigned char ix;
  unsigned char x = 2;
  
  Wire.beginTransmission(slave2w);
  for(ix=0;ix<x;ix++)
  {
    Wire.write(tx_packet[ix]);
  }
  Wire.endTransmission();
}

void OLED::init ()
{
  delay(10);
  Wire.begin();
  delay(10);
    //SPI.begin();
    command(0x2A);  //function set (extended command set)
	command(0x71);  //function selection A, disable internal Vdd regualtor
	data(0x00);
	command(0x28);  //function set (fundamental command set)
	command(0x08);  //display off, cursor off, blink off
	command(0x2A);  //function set (extended command set)
	command(0x79);  //OLED command set enabled
	command(0xD5);  //set display clock divide ratio/oscillator frequency
	command(0x70);  //set display clock divide ratio/oscillator frequency
	command(0x78);  //OLED command set disabled
	command(0x09);  //extended function set (4-lines)
    command(0x06);  //COM SEG direction
	command(0x72);  //function selection B, disable internal Vdd regualtor
	data(0x00);     //ROM CGRAM selection
	command(0x2A);  //function set (extended command set)
	command(0x79);  //OLED command set enabled
	command(0xDA);  //set SEG pins hardware configuration
	command(0x10);  //set SEG pins ... NOTE: When using NHD-0216AW-XB3 or NHD_0216MW_XB3 change to (0x00)
	command(0xDC);  //function selection C
	command(0x00);  //function selection C
	command(0x81);  //set contrast control
	command(0x7F);  //set contrast control
	command(0xD9);  //set phase length
	command(0xF1);  //set phase length
	command(0xDB);  //set VCOMH deselect level
	command(0x40);  //set VCOMH deselect level
	command(0x78);  //OLED command set disabled
	command(0x28);  //function set (fundamental command set)
	command(0x01);  //clear display
	command(0x80);  //set DDRAM address to 0x00
	command(0x0C);  //display ON
  delay(100);
}

  // Set the character insertion address at the given line and character
void OLED::set_point (unsigned char line, unsigned char pos)
{
  command((line * 0x20 + pos) | (1 << 7));
}

// Write the given string (Writes a max of 20 characters)
void OLED::write (const char * str)
{
  unsigned char count = 0;
  while(*str != '\0' && count < 20)
    {
      data(*str);
      count ++;
      str ++;
    }
}
