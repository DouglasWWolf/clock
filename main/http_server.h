#pragma once
#include "common.h"
#include "http_server_base.h"


//=========================================================================================================
// HTTP Server - Handles incoming HTTP requests
//=========================================================================================================
class CHTTPServer : public CHTTPServerBase
{
public:

    // Constructor - just calls the base class
    CHTTPServer(int port = 80) : CHTTPServerBase(port) {}

protected:

    // Called when an HTTP GET is received
    void    on_http_get(const char* resource);

    // Called when an HTTP POST is received
    void    on_http_post(const char* resource);

    // Reply to an HTTP GET of the index page
    void    reply_to_index();
};
//=========================================================================================================



