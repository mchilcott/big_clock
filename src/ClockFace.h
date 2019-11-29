 
#pragma once

extern CRGB leds[];
const unsigned int kNumLEDs = (24*6);

// Number of pixels in each segment
const unsigned int kSegmentLength = 3;

/*
 * 
 * Expected order:
 *  aaa
 * f   b
 * f   b
 *  ggg
 * e   c
 * e   c
 *  ddd
 * 
 */

const unsigned int kDigitStart[] = {0*24,1*24,2*24,3*24,4*24,5*24};


const char char_segment_table[][2] = 
{
    {'0', 0b01111110},
    {'1', 0b00110000},
    {'2', 0b01101101},
    {'3', 0b01111001},
    {'4', 0b00110011},
    {'5', 0b01011011},
    {'6', 0b01011111},
    {'7', 0b01110000},
    {'8', 0b01111111},
    {'9', 0b01110011},
    {'A', 0b01110111},
    {'b', 0b00011111},
    {'C', 0b01001110},
    {'c', 0b00001101},
    {'d', 0b00111101},
    {'E', 0b01001111},
    {'F', 0b01000111},
    {'G', 0b01011110},
    {'g', 0b01111011},
    {'H', 0b00110111},
    {'h', 0b00010111},
    {'I', 0b00000110},
    {'J', 0b01111100},
    {'j', 0b00111100},
    {'K', 0b00000111},
    {'L', 0b00001110},
    {'M', 0b01010100},
    {'n', 0b00010101},
    {'o', 0b00011101},
    {'P', 0b01100111},
    {'q', 0b01110011},
    {'r', 0b00000101},
    {'S', 0b01011011},
    {'t', 0b00001111},
    {'U', 0b00111110},
    {'u', 0b00011100},
    {'v', 0b00100011},
    {'W', 0b00101010},
    {'x', 0b00010100},
    {'Y', 0b00110011},
    {'Z', 0b01101101},
    {'-', 0b00000001},
    {'_', 0b00001000},
    {'(', 0b01001110},
    {')', 0b01111000},
    {'/', 0b00100101},
    {'\\', 0b00010011},
    {'^', 0b01100000},
    {'\'', 0b00100000},
    {'~', 0b01000000},
    {'`', 0b00000010},
    {'?', 0b01100101}
};


/**
 * \param digit_offset starting address of the 7-segment group in the mimic buffer
 * \param segments should be of the form 0abcdefg in binary where each letter represents the state of the character (1=on)
 */
void set_segment_display (CRGB *mimic, unsigned int digit_offset, byte segments, CRGB colour, CRGB off_colour = {0,0,0})
{
    for (int segment = 6; segment >= 0; -- segment)
    {
        unsigned int segment_offset = digit_offset + segment * kSegmentLength;
        
        if (segments & 0x01)
        {
            for(unsigned int i = 0; i < kSegmentLength; ++i)
                mimic[segment_offset + i] = colour;
        }
        else
        {
            for(unsigned int i = 0; i < kSegmentLength; ++i)
                mimic[segment_offset + i] = off_colour;
        }
        
        segments >>= 1;
    }
}

void set_colon(CRGB *mimic, unsigned int digit_offset, CRGB colour)
{
    mimic[digit_offset + 22] = colour;
    mimic[digit_offset + 23] = colour;
    
}

void set_decimal(CRGB *mimic, unsigned int digit_offset, CRGB colour)
{
    mimic[digit_offset + 21] = colour;
}


/**
 * Get the nth character of a decimal number (as an int, not ascii)
 * 
 */
byte get_decimal_char(int number, byte character)
{
    while(character > 0)
        {
            number /= 10;
            --character;
        }
        
    return number % 10;
}

/**
 * Find the segment representation for a given character
 */
byte get_rep(const char input)
{
    for(int i = 0; i < sizeof(char_segment_table); ++i){
        if(char_segment_table[i][0] == input)
            return char_segment_table[i][1];
    }
    return 0;
}

