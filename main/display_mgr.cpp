//=========================================================================================================
// display_mgr.cpp - Implements a thread that decides what will be displayed on the 7-Segment display
//=========================================================================================================
#include "display_mgr.h"
#include "globals.h"

#define FOREVER 0xFFFFFFFF

enum qentry_t : unsigned char
{
    DISPLAY_NOW,
    WAITING_FOR_FLIP,
    DISPLAY_IP0,
    DISPLAY_IP1,
    DISPLAY_IP2,
    DISPLAY_IP3
};
qentry_t current_mode = DISPLAY_NOW;


//=========================================================================================================
// display_current_time() - Displays the current wall-clock time on the display
//=========================================================================================================
static void display_current_time()
{
    struct tm timeinfo;

    // If we're in Wi-Fi AP mode, don't display a time
    if (Network.wifi_status() == WIFI_AP_MODE)
    {
        Display.show_string(" AP ");
        return;
    }

    // Find out the current time and date
    time_t now = time(NULL);

    // Break that out into hour, mininute, and second fields
    localtime_r(&now, &timeinfo);

    // Convert military time into conventional time
    if (timeinfo.tm_hour > 12) timeinfo.tm_hour -= 12;
    if (timeinfo.tm_hour == 0) timeinfo.tm_hour = 12; 

    // Display the time on the physical display
    Display.show_time(timeinfo.tm_hour, timeinfo.tm_min);

    // Print the time to stdout to aid in debugging
    printf(">>> %2i:%02i <<<\n", timeinfo.tm_hour, timeinfo.tm_min);
}
//=========================================================================================================


//=========================================================================================================
// display_ip_octet() - Displays a single octet of our IP address
//=========================================================================================================
static void display_ip_octet(int n)
{
    // Display the characters on the phyiscal display
    Display.show_number(n);

    // Print the number to stdout to aid in debugging
    printf(">>> %i <<<\n", n);
}
//=========================================================================================================


//=========================================================================================================
// ip_octet() - Returns a pointer to the ASCII representation of the specified IP address octet
//=========================================================================================================
static int ip_octet(int index)
{
    char buffer[10];
    
    // Point to the buffer where we're going to store the ASCII octet
    char *out = buffer;

    // Point to an ASCII representation of our IP address
    const char* in = System.ip_addr;

    // Skip past the correct number of decimal points
    while (index--) in = strchr(in, '.')+1;        

    // Copy the octet into the buffer
    while (*in && *in != '.') *out++ = *in++;

    // Nul-terminate the buffer
    *out = 0;

    // And hand the octet to the caller
    return atoi(buffer);
}
//=========================================================================================================



//=========================================================================================================
// seconds_until_flip() - The number of seconds until a new minutes starts
//=========================================================================================================
static int seconds_until_flip()
{
    struct tm timeinfo;

    // Find out the current time and date
    time_t now = time(NULL);

    // Break that out into hour, mininute, and second fields
    localtime_r(&now, &timeinfo);

    // Tell the caller how many seconds until a new minutes starts
    return 60-timeinfo.tm_sec;
}
//=========================================================================================================


//=========================================================================================================
// launch_task() - Calls the "task()" routine in the specified object
//
// Passed: *pvParameters points to the object that we want to use to run the task
//=========================================================================================================
static void launch_task(void *pvParameters)
{
    // Fetch a pointer to the object that is going to run out task
    CDisplayMgr* p_object = (CDisplayMgr*) pvParameters;
    
    // And run the task for that object!
    p_object->task();
}
//=========================================================================================================


//=========================================================================================================
// start() - Starts the task
//=========================================================================================================
void CDisplayMgr::start()
{
    // If we're already started, do nothing
    if (m_task_handle) return;

    // Create the queue that the ISR will post event messages to when an interrupt occurs
    m_event_queue = xQueueCreate(10, sizeof(qentry_t));

    // How long we should wait for an incoming event
    m_wait_time_ms = FOREVER;
    
    printf(">>> Starting Display Manager <<<\n");

    // Create the task
    xTaskCreatePinnedToCore(launch_task, "display_mgr", 3000, this, DEFAULT_TASK_PRI, &m_task_handle, TASK_CPU);
}
//=========================================================================================================


//=========================================================================================================
// display_now() - Displays the time right now then goes into "WAITING_FOR_FLIP" mode
//=========================================================================================================
void CDisplayMgr::display_now()
{
    qentry_t cmd = DISPLAY_NOW;
    xQueueSend(m_event_queue, &cmd, 0);
}
//=========================================================================================================




//=========================================================================================================
// display_ip_address() - Displays the system IP address, one octet at a time
//=========================================================================================================
void CDisplayMgr::display_ip_address()
{
    qentry_t cmd = DISPLAY_IP0;
    xQueueSend(m_event_queue, &cmd, 0);
}
//=========================================================================================================



//=========================================================================================================
// task() - This is the thread that manages what's currently displayed on the 7-segment display
//=========================================================================================================
void CDisplayMgr::task()
{
    qentry_t cmd;

    while (true)
    {
        // If the current mode is "Wait for the minute to change", determine how long to wait
        if (current_mode == WAITING_FOR_FLIP)
        {
            int seconds = seconds_until_flip() - 2;
            m_wait_time_ms = (seconds > 0) ? seconds * 1000 : 500;
        }

        // Determine how many timer ticks to wait for
        TickType_t ticks = (m_wait_time_ms == FOREVER) ? portMAX_DELAY : m_wait_time_ms / portTICK_PERIOD_MS;

        // If we received a new message from the event queue, switch to that mode
        if (xQueueReceive(m_event_queue, &cmd, ticks)) current_mode = cmd;
        
        // Do whatever it correct for the current mode
        switch(current_mode)
        {
            case DISPLAY_NOW:
                display_current_time();
                current_mode = WAITING_FOR_FLIP;
                break;            

            case WAITING_FOR_FLIP:
                if (seconds_until_flip() > 55) display_current_time();
                break;
            
            case DISPLAY_IP0:
                display_ip_octet(ip_octet(0));
                m_wait_time_ms = 1000;
                current_mode = DISPLAY_IP1;
                break;

            case DISPLAY_IP1:
                display_ip_octet(ip_octet(1));
                m_wait_time_ms = 1000;
                current_mode = DISPLAY_IP2;
                break;
            
            case DISPLAY_IP2:
                display_ip_octet(ip_octet(2));
                m_wait_time_ms = 1000;
                current_mode = DISPLAY_IP3;
                break;
            
            case DISPLAY_IP3:
                display_ip_octet(ip_octet(3));
                m_wait_time_ms = 1000;
                current_mode = DISPLAY_NOW;
                break;

            default:
                break;
        }

    }

}
//=========================================================================================================
