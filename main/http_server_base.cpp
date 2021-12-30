#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/param.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <stdint.h>
#include <stdarg.h>
#include "globals.h"

static const char* TAG = "http_server";

//=========================================================================================================
// Constructor() 
//=========================================================================================================
CHTTPServerBase::CHTTPServerBase(int port)
{
    m_task_handle = nullptr;
    m_sock = CLOSED;
    m_has_client = false;
    m_server_port = port;
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
    CHTTPServerBase* p_object = (CHTTPServerBase*) pvParameters;
    
    // And run the task for that object!
    p_object->task();
}
//=========================================================================================================


//=========================================================================================================
// start() - Starts the TCP server task
//=========================================================================================================
void CHTTPServerBase::start()
{
    // If we're already started, do nothing
    if (m_task_handle) return;

    // Create the task
    xTaskCreatePinnedToCore(launch_task, "tcp_server", 3000, this, TASK_PRIO_TCP, &m_task_handle, TASK_CPU);
}
//=========================================================================================================


//=========================================================================================================
// stop() - Stops the TCP server task
//=========================================================================================================
void CHTTPServerBase::stop()
{
    static TaskHandle_t task_handle;

    // Kill the task if it's running
    if (m_task_handle)
    {
        task_handle = m_task_handle;
        m_task_handle = nullptr;
        vTaskDelete(task_handle);
    }

    // If this was called from a different thread, we'll get a chance to close the sockets
    // This has to come <<after>> the vTaskDelete() because trying to close a socket that is in
    // active use can either hang or panic the system.
    hard_shutdown();
}
//=========================================================================================================



//=========================================================================================================
// execute() - State machine: Call this often, or with blocking set to true
// 
// If blocking is true:
//     Virtual function read() should block until it has data to return
//     If the input stream closes, read() should return -1
//     If read() returns -1, this routine will return
//=========================================================================================================
void CHTTPServerBase::execute()
{
    char c;

again:

    // Keep track of the high-water mark on the stack for this thread
    StackMgr.record_hwm(TASK_IDX_HTTP_SERVER);

    // This is how many bytes we have remaining free in buffer that holds the incoming message
    int free_remaining = sizeof(m_message) - 1;

    // This is where the next incoming character will be stored
    char* p_input = m_message;

     // For each character available
    while (true)
    {
        // Fetch a single character from the socket;
        if (recv(m_sock, &c, 1, 0) < 1) break;

        // Throw away carriage returns
        if (c == 13) continue;

        // Handle linefeed
        if (c == 10)
        {
            // Nul-terminate the message
            *p_input = 0;

            // If this was a blank line, go handle the request
            if (p_input == m_message)
            {
                // If there is content to still be received, fetch it
                if (m_request_content_length)
                {
                    ::recv(m_sock, m_request_content, m_request_content_length, 0);
                    m_request_content[m_request_content_length] = 0;                  
                }

                if (m_request_type == GET)
                    on_http_get(m_request_resource);
                else if (m_request_type == POST)
                    on_http_post(m_request_resource);
                goto again;
            }

            // Keep track of which input line this is
            ++m_line_number;

            // If this is the first line, we need to parse out the GET/POST command
            if (m_line_number == 1) 
            {
                parse_first_line();
                goto again;
            }

            // If this line is the content length, record it
            if (strncmp(m_message, "Content-Length:", 15) == 0)
            {
                m_request_content_length = atoi(m_message + 15);
                goto again;
            }

            // Reset back to an empty message buffer
            goto again;
        }

        // If there's room to add this character to the input buffer, make it so
        if (free_remaining)
        {
            *p_input++ = c;
            --free_remaining;
        }
    }
}
//=========================================================================================================

  





//========================================================================================================= 
// wait_for_connection() - Closes down any existing socket, creates a new one, and starts listening for 
//                         a TCP connection on our server port
//
// Returns:  'true' if a socket is succesfully created and a client connected
//           'false' if something went awry in the socket-creation process.
//
// On Entry: is_socket_created = true if a socket already exists
//
// On Exit:  sock = socket descriptor of a socket that has a client connected to it
//========================================================================================================= 
bool CHTTPServerBase::wait_for_connection()
{
    int error, True = 1;
    struct sockaddr_in sock_desc;

    // We have no client connected
    m_has_client = false;

    // If we already have a socket created, close it down
    hard_shutdown();

    // We can bind to any available IP address (though there will really only be one)
    sock_desc.sin_addr.s_addr = htonl(INADDR_ANY);

    // This is an IP socket
    sock_desc.sin_family = AF_INET;

    // Bind the the pre-defined port number
    sock_desc.sin_port = htons(m_server_port);

    // Create our socket
    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    // This socket is allowed to re-use a previous bound port number
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&True, sizeof(True));

    // Bind the socket to the TCP port we specified
    error = bind(m_sock, (struct sockaddr *)&sock_desc, sizeof(sock_desc));
    
    // If that bind failed, it's a fatal error
    if (error)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        return false;
    }

    // Begin listening for TCP connections on our predefined port
    error = listen(m_sock, 1);
    
    // If that somehow failed, it's a fatal error
    if (error)
    {
        ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
        return false;
    }

    // The IP address of the client will be stored here
    struct sockaddr_in6 source_addr; 
    uint32_t addr_len = sizeof(source_addr);

    // Wait for a client connect and accept it when it arrives
    int new_socket = accept(m_sock, (struct sockaddr *)&source_addr, &addr_len);

    // Close the original socket we were listening on
    close(m_sock);

    // And start using the new socket
    m_sock = new_socket;

    // We now have a client connected
    m_has_client = true;

    // Initialize information about the HTTP request we're about to receive
    m_line_number = 0;
    m_request_type = UNKNOWN;
    m_request_resource[0] = 0;
    m_request_content[0] = 0;
    m_request_content_length = 0;

    // Tell the caller that all is well
    return true;
}
//========================================================================================================= 






//========================================================================================================= 
// hard_shutdown() - Forces the socket closed
//========================================================================================================= 
void CHTTPServerBase::hard_shutdown()
{
    if (m_sock != CLOSED)
    {
        shutdown(m_sock, 0);
        close(m_sock);
        m_sock = CLOSED;
    }
}
//========================================================================================================= 






//========================================================================================================= 
// task() - When the thread is spawned, this is the routine that starts in its own thread
//========================================================================================================= 
void CHTTPServerBase::task()
{
    // Ensure that sockets are closed
    hard_shutdown();

    // We're going to do this forever
    while (true)
    {
        // Build our listening socket and wait for a client to connect.
        // If something goes awry, there's no way to recover, so we halt this task
        if (!wait_for_connection()) stop();

        // Fetch and handle incoming messages
        execute();
    }
}
//========================================================================================================= 




//========================================================================================================= 
// set_nagling() - Turns on and off Nagle's Algorithm.  If Nagling is off, all packets will be sent to the
//                 interface immediately upon  completion of a "::send()" operation
//========================================================================================================= 
void CHTTPServerBase::set_nagling(bool flag)
{
    // Set up our flag that will be passed to setsockopt()
    char value = flag ? 1 : 0;

    // And set the Nagling option appropriately
    setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, &value, sizeof value);
}
//========================================================================================================= 



//========================================================================================================= 
// parse_first_line() - Parses the first line of the HTTP request
//========================================================================================================= 
void CHTTPServerBase::parse_first_line()
{
    // Skip over the first token
    char* in = m_message;
    while (*in != ' ' && *in != 0) ++in;

    // Nul terminate the first token
    *in++ = 0;

    // Find out what kind of HTTP request this is
    if (strcmp(m_message, "GET") == 0)
        m_request_type = GET;
    else if (strcmp(m_message, "POST") == 0)
        m_request_type = POST;
    else return;

    // Skip past any spaces
    while (*in == ' ') ++in;

    // This is where the resource name (usually a URL) starts
    const char* start = in;

    // Skip past the token, and nul-terminate
    while (*in != ' ' && *in != 0) ++in;
    *in = 0;

    // Extract the name of the requested resource
    strcpy(m_request_resource, start);
}
//========================================================================================================= 



//========================================================================================================= 
// reply() - Sends a complete HTTP response and closes the socket
//========================================================================================================= 
void CHTTPServerBase::reply(int code, const char* content)
{
    char buffer[100];
    
    const char response[] = "HTTP/1.1 %i OK\r\nContent-Type: text/html\r\nContent-Length: %i\r\n\r\n";

    // This is how long the content is
    int content_length = strlen(content);

    // Format the response header
    sprintf(buffer, response, code, content_length);

    // Send the response header
    ::send(m_sock, buffer, strlen(buffer), 0);

    // And send the content if there is any
    if (content_length) ::send(m_sock, content, content_length, 0);

    // And close the socket
    close(m_sock);
    m_sock = -1;
}
//========================================================================================================= 
