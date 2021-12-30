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

    // Reply to an HTTP GET /
    void    reply_to_index();

    // Reply to an HTTP POST /config
    void    reply_to_config();

    // Save the updated configuration
    void    save_updated_config();
};
//=========================================================================================================



