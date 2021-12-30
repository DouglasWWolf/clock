//=========================================================================================================
// webpage.cpp - Implements a managed text buffer for building webpages
//=========================================================================================================
#include "webpage.h"
#include <stdarg.h>
#include <stdio.h>

//=========================================================================================================
// add() - Adds raw text to the page buffer
//=========================================================================================================
void CWebpage::add(const char* text)
{
    // And copy it into our page buffer
    while (*text) *m_ptr++ = *text++;

    // Always nul-terminate the page buffer
    *m_ptr = 0;    
}
//=========================================================================================================



//=========================================================================================================
// addf() - Adds formatted text to the page buffer
//=========================================================================================================
void CWebpage::addf(const char* fmt, ...)
{
    va_list ap;

    // Format the the data into "buffer"
    va_start(ap, fmt);
    int count = vsprintf(m_ptr, fmt, ap);
    va_end(ap);

    // Bump our pointer to the end of the buffer
    m_ptr += count;

    // Always nul-terminate the page buffer
    *m_ptr = 0;    
}
//=========================================================================================================
