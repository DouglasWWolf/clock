//=========================================================================================================
// display_mgr.h - Defines a thread that decides what will be displayed on the 7-Segment display
//=========================================================================================================
#pragma once
#include "common.h"

class CDisplayMgr
{
public:

    // Starts the thread
    void    start();

    // Call this to display the time right now
    void    display_now();

    // Call this to display the system IP address
    void    display_ip_address();

public:      

    // When the thread spawns, this is the routine that starts 
    void    task();

protected:

    // This is the index (0 thru 3) of the next octet of the IP address to display
    int             m_ip_octet;
    
    // This is the handle of the currently running server task
    TaskHandle_t    m_task_handle;

    // Events in this queue determine what gets displayed
    QueueHandle_t   m_event_queue;

    // How long we wait for an event to arrive in the queue
    uint32_t        m_wait_time_ms;
};


