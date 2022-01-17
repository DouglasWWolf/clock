//=========================================================================================================
// ht16k33.h - Defines an interface to an HT16K33 LED matrix driver
//=========================================================================================================
#pragma once


class CHT16K33
{
public:

    // Call once at boot to initialize
    void    init(int i2c_address);

    // Displays the time with two dots between hour and minute
    void    show_time(int hour, int minute);

    // Displays a positive integer of 1 to 4 characters
    void    show_number(int n);

    // Displays a 4 character string on the display
    void    show_string(const char* s);

    // Display a blinking dot to show we're connecting to the router
    void    show_wait_for_router();

    // Display a blinking colon to show we're waiting for SNTP
    void    show_wait_for_ntp();

    // Call this to change the display brightness.  "Level" should be 0 thru 15
    void    set_brightness(int level);

protected:
    int     m_i2c_address;

};