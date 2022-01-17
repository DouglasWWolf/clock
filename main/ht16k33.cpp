//=========================================================================================================
// ht16k33.cpp - Implements an interface to an HT16K33 LED matrix driver
// 
// 1st digit is at address 0
// 2nd digit is at address 2
// 3rd digit is at address 6
// 4th digit is at address 8
//
// Address 4:
//   Bit 1 = colon in the center
//   Bit 2 = top leftmost dot
//   Bit 3 = bottom leftmost dot
//   Bit 4 = Upper rightmost dot
//=========================================================================================================
#include "ht16k33.h"
#include "globals.h"



#define CMD_CONFIG      0x81 
#define CMD_BRIGHTNESS  0xE0 

#define BLINK_OFF      0     
#define BLINK_2HZ      1     
#define BLINK_1HZ      2     
#define BLINK_HALFHZ   3     


static const unsigned char font[] =
{
    0b00000000, // (space)
    0b10000110, // !
    0b00100010, // "
    0b01111110, // #
    0b01101101, // $
    0b11010010, // %
    0b01000110, // &
    0b00100000, // '
    0b00101001, // (
    0b00001011, // )
    0b00100001, // *
    0b01110000, // +
    0b00010000, // ,
    0b01000000, // -
    0b10000000, // .
    0b01010010, // /
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00001001, // :
    0b00001101, // ;
    0b01100001, // <
    0b01001000, // =
    0b01000011, // >
    0b11010011, // ?
    0b01011111, // @
    0b01110111, // A
    0b01111100, // B
    0b00111001, // C
    0b01011110, // D
    0b01111001, // E
    0b01110001, // F
    0b00111101, // G
    0b01110110, // H
    0b00110000, // I
    0b00011110, // J
    0b01110101, // K
    0b00111000, // L
    0b00010101, // M
    0b00110111, // N
    0b00111111, // O
    0b01110011, // P
    0b01101011, // Q
    0b00110011, // R
    0b01101101, // S
    0b01111000, // T
    0b00111110, // U
    0b00111110, // V
    0b00101010, // W
    0b01110110, // X
    0b01101110, // Y
    0b01011011, // Z
    0b00111001, // [
    0b01100100, //
    0b00001111, // ]
    0b00100011, // ^
    0b00001000, // _
    0b00000010, // `
    0b01011111, // a
    0b01111100, // b
    0b01011000, // c
    0b01011110, // d
    0b01111011, // e
    0b01110001, // f
    0b01101111, // g
    0b01110100, // h
    0b00010000, // i
    0b00001100, // j
    0b01110101, // k
    0b00110000, // l
    0b00010100, // m
    0b01010100, // n
    0b01011100, // o
    0b01110011, // p
    0b01100111, // q
    0b01010000, // r
    0b01101101, // s
    0b01111000, // t
    0b00011100, // u
    0b00011100, // v
    0b00010100, // w
    0b01110110, // x
    0b01101110, // y
    0b01011011, // z
    0b01000110, // {
    0b00110000, // |
    0b01110000, // }
    0b00000001, // ~
    0b00000000, // del
};


//=========================================================================================================
// init() - Called once to initialize the device
//=========================================================================================================
void CHT16K33::init(int i2c_address)
{
    // Save the I2C address for posterity
    m_i2c_address = i2c_address;

    // Turn on the oscillator
    if (I2C.write(m_i2c_address, 0x21, 1))
        printf(">>> HT16K33 IS ALIVE!!!! <<<\n");
    else
        printf(">>> HT16K33 NOT FOUND !!! <<<\n");

    I2C.write(m_i2c_address, 0x21, 1);   // Oscillator on
    I2C.write(m_i2c_address, 0xA0, 1);   // INT/ROW pin is a row driver
    
    for (int n=0; n<6; ++n) I2C.write(m_i2c_address, n*2, 1, 0x0000, 2);
    
    I2C.write(m_i2c_address, 0x81, 1);   // Display on, no blinking
    I2C.write(m_i2c_address, 0xEF, 1);   // Max brightness
}
//=========================================================================================================


//=========================================================================================================
// show_time() - Displays the current time
//=========================================================================================================
void CHT16K33::show_time(int hour, int minute)
{
    char buffer[17];
    const int DIGIT_OFFSET = 16;

    printf(">>>> show_time(%i, %i)\n", hour, minute);
    memset(buffer, 0, sizeof buffer);

    // Compute the offsets into the font table for each digit
    int c1 = (hour > 9) ? DIGIT_OFFSET + 1 : 0;
    int c2 = DIGIT_OFFSET + hour % 10;
    int c3 = DIGIT_OFFSET + minute / 10;
    int c4 = DIGIT_OFFSET + minute % 10;

    // Place each digit into the data buffer
    buffer[1] = font[c1];
    buffer[3] = font[c2];
    buffer[5] = 2;
    buffer[7] = font[c3];
    buffer[9] = font[c4];

    // Write the 16 bytes of data to address 0 in the device
    I2C.write(m_i2c_address, buffer, sizeof buffer);
}
//=========================================================================================================


//=========================================================================================================
// show_number() - Displays a 1 to 4 digit number
//=========================================================================================================
void CHT16K33::show_number(int n)
{
    char buffer[17], ascii[10];
    
    // Fetch an ASCII representation of the number
    sprintf(ascii, "%4i", n);
    
    // Clear the buffer we're going to send to the device
    memset(buffer, 0, sizeof buffer);

    // Compute the offsets into the font table for each digit
    int c1 = ascii[0] - 32;
    int c2 = ascii[1] - 32;
    int c3 = ascii[2] - 32;
    int c4 = ascii[3] - 32;

    // Place each digit into the data buffer
    buffer[1] = font[c1];
    buffer[3] = font[c2];
    buffer[7] = font[c3];
    buffer[9] = font[c4];

    // Write the 16 bytes of data to address 0 in the device
    I2C.write(m_i2c_address, buffer, sizeof buffer);
}
//=========================================================================================================



//=========================================================================================================
// show_string() - Displays a 4 character string on the display
//=========================================================================================================
void CHT16K33::show_string(const char* ascii)
{
    char buffer[17];
   
    // Clear the buffer we're going to send to the device
    memset(buffer, 0, sizeof buffer);

    // Compute the offsets into the font table for each digit
    int c1 = ascii[0] - 32;
    int c2 = ascii[1] - 32;
    int c3 = ascii[2] - 32;
    int c4 = ascii[3] - 32;

    // Place each digit into the data buffer
    buffer[1] = font[c1];
    buffer[3] = font[c2];
    buffer[7] = font[c3];
    buffer[9] = font[c4];

    // Write the 16 bytes of data to address 0 in the device
    I2C.write(m_i2c_address, buffer, sizeof buffer);
}
//=========================================================================================================





//=========================================================================================================
// set_brightness() - Sets the display brightness
//=========================================================================================================
void CHT16K33::set_brightness(int level)
{
    if (level < 0) level = 0;
    if (level > 15) level = 15;
    I2C.write(m_i2c_address, CMD_BRIGHTNESS | level, 1);   
}
//=========================================================================================================
