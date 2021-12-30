//=========================================================================================================
// webpage.h - Definition for 'CWebpage' class
//=========================================================================================================
#pragma once

class CWebpage
{
public:

    // Constructor, builds an empty page
    CWebpage() {start();}

    // Call this to start a new page
    void    start() {m_ptr = m_page; m_page[0] = 0;}

    // Call this to add text
    void    add(const char* text);

    // Call this to add formatted text
    void    addf(const char* fmt, ...);

    // Overloading operator '+=' for convenience
    void operator += (const char* text) { add(text); }

    // Call this to get a reference to the text of the webpage
    char* text() {return m_page;}


private:

    // This is the buffer where the page will be composed
    char m_page[5 * 1024];
    
    // This is a pointer to where the next bytes should be written
    char* m_ptr;
    
};



