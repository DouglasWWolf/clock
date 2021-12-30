//=========================================================================================================
// http_server_base.h - The base class for an HTTP server
//=========================================================================================================
#pragma once
#include "common.h"

class CHTTPServerBase
{

    //--------------------------------------------------------------------------------
    //  Public API for the outside world to access
    //--------------------------------------------------------------------------------
public:

    // Constructor
    CHTTPServerBase(int port = 80);

    // Starts the thread that runs the server
    void    start();

    // Call this to delete the thread that is running the server
    void    stop();

    // Call this to turn Nagle's algorithm on or off.  "false" means "send packets immediately"
    void    set_nagling(bool flag);

    // Call this to find out if there is a client connected to our server
    bool    has_client() {return m_has_client;}

    //--------------------------------------------------------------------------------
    // Public only so that launch_thread() has access to it
    //--------------------------------------------------------------------------------
public:      

    // When the thread spawns, this is the routine that starts 
    void    task();


    //--------------------------------------------------------------------------------
    // Over-ride these
    //--------------------------------------------------------------------------------
    virtual void  on_http_get (const char* resource) = 0;
    virtual void  on_http_post(const char* resource) = 0;

    //--------------------------------------------------------------------------------
    // Tools for request handlers to use
    //--------------------------------------------------------------------------------
protected:      

    // Keeps track of the type of HTTP request
    enum http_t {GET, POST, UNKNOWN} m_request_type;

    // This is the URL that is the subject of the HTTP request
    char    m_request_resource[128];

    // The content of the HTTP request
    char    m_request_content[1024];

    // The length of m_request_content
    int     m_request_content_length;

    // Call this to send a reply to an HTTP POST or HTTP GET
    void    reply(int code, const char* content = "");

    //--------------------------------------------------------------------------------
    // Functions and data that are private to the base class
    //--------------------------------------------------------------------------------
private:

    // Create a socket and listen for connections
    bool    wait_for_connection();

    // Once a connection is made, this executes the command handler
    void    execute();

    // Parses the first line of an HTTP request
    void    parse_first_line();

    // This is our incoming message
    char    m_message[128];
    
    // This is the line number of the HTTP request that we are processing
    int     m_line_number;



private:  /* TCP and ESP specific stuff */


    // Forces the socket closed if it's open
    void    hard_shutdown();

    const int CLOSED = -1;

    // This is the handle of the currently running server task
    TaskHandle_t    m_task_handle;

    // This is the socket descriptor of the TCP socket
    int             m_sock;

    // This will be true if there is a client connected
    bool            m_has_client;

    // This is the server port we listen on
    int             m_server_port;

};

