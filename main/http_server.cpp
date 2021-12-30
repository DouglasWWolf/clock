//=========================================================================================================
// http_server.cpp - Implements our web server
//=========================================================================================================
#include "webpage.h"
#include "globals.h"
#include "history.h"

static CWebpage webpage;

//=========================================================================================================
// html_style[] - The heading of an HTML webpage, defining our style-sheet
//=========================================================================================================
static const char html_style[] =

    "<!DOCTYPE html><html>"
    "<head><style>"
        "table"
        "{"
            "display:flex;"
            "justify-content: center;"
            "padding: 1%;"
            "border-collapse: collapse;"
            "outline: none;"
        "}"

        "th"
        "{"
            "padding: 5px;"
            "width : 160;"
        "}"

        "td"
        "{"
            "border: 2px solid black;"
            "padding: 1%;"
            "outline: none;"
        "}"

        "input"
        "{"
            "width:95%;"
            "background-color: #E0E0E0;"
            "type: text;"
            "font-size: 15px;"
        "}"

        "button"
        "{"
            "display: flex;"
            "justify-content:center;"
            "margin:0 auto;"
            "width:40%;"
            "border:2px solid black;"
            "text-align:center;"
            "border-radius:12px;"
            "font-size: 25px;"
            "padding:15px;"
        "}"

        ".button:hover"
        "{"
            "background-color: #C0C0C0;"
        "}"

        "h1"
        "{"
            "text-align: center;"
        "}"
        
        "</style></head>";
//=========================================================================================================

//=========================================================================================================
// html_final[] - Closes out the body of an HTML webpage
//=========================================================================================================
static const char html_final[] = "</body></html>";
//=========================================================================================================


//=========================================================================================================
// script_on_main_screen_button[] - JavaScript function "on_main_screen_button()"
//=========================================================================================================
static const char script_on_main_screen_button[] = 
    "<script>\n"
    "function on_main_screen_button(post_uri)\n"
    "{\n"
        "var btn1=document.getElementById('brt_button');\n"
        "var btn2=document.getElementById('dim_button');\n"
        "var btn3=document.getElementById('config_button');\n"
        "var btn4=document.getElementById('reboot_button');\n"
      
        "btn1.style.background='#808080';\n"
        "btn2.style.background='#808080';\n"
        "btn3.style.background='#808080';\n"
        "btn4.style.background='#808080';\n"
        
        "btn1.disable=true;\n"
        "btn2.disable=true;\n"
        "btn3.disable=true;\n"
        "btn4.disable=true;\n"
                
        "var xhr=new XMLHttpRequest();\n"
        "xhr.onreadystatechange=function()\n"
        "{\n"
            "if(xhr.readyState===4)\n"
            "{\n"
                "if(xhr.status !=200)\n"
                "{\n"
                    "location.replace('/');\n"
                    "return;\n"
                "}\n"
            "}\n"
        "};\n"
        "xhr.open('POST',post_uri,true);\n"
        "xhr.setRequestHeader('Content-Type','application/json');\n"
        "xhr.send();\n"
    "}\n"
    "</script>\n";
//=========================================================================================================



//=========================================================================================================
// script_on_reboot[] - Javascript to handle the "reboot" button
//=========================================================================================================
static const char script_on_reboot[] = 
    "<script>\n"
    "function on_reboot()\n"
    "{\n"
        "document.getElementById('reboot_button').disable=true;\n"
        "var xhr=new XMLHttpRequest();\n"
        "xhr.onreadystatechange=function()\n"
        "{\n"
            "if(xhr.readyState===4)\n"
            "{\n"
                "if(xhr.status==201)\n"
                "{\n"
                    "location.reload();\n"
                    "return;\n"
                "}\n"
                "alert('Wait 20 seconds while the system reboots, then click OK');\n"
                "window.location='/';\n"
            "}\n"
        "}\n"
        "xhr.open('POST','/reboot',true);\n"
        "xhr.setRequestHeader('Content-Type','application/json');\n"
        "xhr.send();\n"
    "}\n"
    "</script>\n";
//=========================================================================================================


//=========================================================================================================
// JavaScript code for functions on_brighter() and on_dimmer()
//=========================================================================================================
static const char script_on_brighter[] =
    "<script>\n"
    "function on_brighter()\n"
    "{\n"
        "on_main_screen_button('/brighter');\n"
    "}\n"
    "</script>\n";

static const char script_on_dimmer[] =
    "<script>\n"
    "function on_dimmer()\n"
    "{\n"
        "on_main_screen_button('/dimmer');\n"
    "}\n"
    "</script>\n";
//=========================================================================================================



//=========================================================================================================
// script_on_config[] - JavaScript function "on_config()"
//=========================================================================================================
static const char script_on_config[] = 
    "<script>\n"
    "function on_config()\n"
    "{\n"
        "var xhr=new XMLHttpRequest();\n"
        "xhr.open('GET','/config', true);\n"
        "xhr.setRequestHeader('Content-Type','application/json');\n"
        "xhr.send();\n"
    "}\n"
    "</script>\n";
//=========================================================================================================



//=========================================================================================================
// script_on_config_save[] - JavaScript function "on_config_save()"
//=========================================================================================================
static const char script_on_config_save[] =
    "<script>\n"
    "function on_config_save()\n"
    "{\n"
        "var result='';\n"
        "document.getElementById('save_button').disable=true;\n"
        "document.getElementById('exit_button').disable=true;\n"
        "result+=';netssid='+document.getElementById('netssid').value;\n"
        "result+=';netpw='+document.getElementById('netpw').value;\n"
        "result+=';utc_offset='+document.getElementById('utc_offset').value;\n"

        "var xhr=new XMLHttpRequest();\n"
        "xhr.onreadystatechange=function()\n"
        "{\n"
            "if(xhr.readyState===4)\n"
            "{\n"
                "location.assign('/');\n"
/*
                "if(xhr.status==200)\n"
                "{\n"
                    "location.assign('/');\n"
                    "return;\n"
                "}\n"
                "document.getElementById('save_button').disable=false;\n"
                "document.getElementById('exit_button').disable=false;\n"
                "location.assign('/');\n"
*/
                "return;\n"
            "}\n"
        "}\n"
        "xhr.open('POST','/updatecfg',true);\n"
        "xhr.setRequestHeader('Content-Type','application/json');\n"
        "xhr.send(result);\n"
    "}\n"
    "</script>\n";
//=========================================================================================================






//=========================================================================================================
// reply_to_index() - Replies to an "HTTP GET /""
//=========================================================================================================
static const char html_index_01[] = 

    "<body>"
    "<h1>Doug's Clock</h1>"
    "<table><tr><th>Firmware: %s</th><th> &nbsp &nbsp </th><th>RSSI: %i</th></tr></table>";

static const char html_index_02[] =
	"<br><br>"
    "<button class='button' id='brt_button' onclick='on_brighter()'>Brighter</button><br>"
    "<button class='button' id='dim_button' onclick='on_dimmer()'>Dimmer</button><br>"
    "<br><br><br>"
    
    "<form action='/config' method='post'>"
    "<button class='button' type='submit', id='config_button'>Configure</button><br>"
    "</form>"

    "<button class='button' id='reboot_button' onclick='on_reboot()'>Reboot Device</button>";



void CHTTPServer::reply_to_index()
{
    webpage.start();
    webpage += html_style;
    webpage.addf(html_index_01, FW_VERSION, System.rssi());
    webpage += html_index_02;
    webpage += script_on_reboot;
    webpage += script_on_main_screen_button;
    webpage += script_on_brighter;
    webpage += script_on_dimmer;
    webpage += script_on_config;
    webpage += html_final;
    reply(200, webpage.text());
}
//=========================================================================================================


//=========================================================================================================
// reply_to_config() - Replies to an "HTTP POST /config""
//=========================================================================================================
static const char html_config_01[] =  "<body><h1>Doug's Clock</h1>";

static const char html_config_02[] = 
    "<table>"
    "<tr>"
    "<th>Network SSID</th>"
    "<th>Network Password</th>"
    "<th>UTC +/-</th>"
    "</tr>";


static const char html_config_03[] =
    "<br><button class='button' id='save_button' onclick='on_config_save()'>Save Changes</button><br><br>"
    "<form action='/' method='post'>"
    "<button class='button' type='submit', id='exit_button'>Exit Without Saving</button><br>"
    "</form>";

void CHTTPServer::reply_to_config()
{
    webpage.start();
    webpage += html_style;
    webpage += html_config_01;
    webpage += html_config_02;
    webpage += "<tr>";
    webpage.addf("<td><input id='netssid'    value ='%s' autofocus</td>", NVS.data.network_ssid);
    webpage.addf("<td><input id='netpw'      value ='%s'</td>", NVS.data.network_pw);
    webpage.addf("<td><input id='utc_offset' value ='%i'</td>", NVS.data.utc_offset);
    webpage += "</tr>";
    webpage += "</table><br><br>";
    webpage += html_config_03;
    webpage += script_on_config_save;
    webpage += html_final;

    reply(200, webpage.text());
}
//=========================================================================================================






//=========================================================================================================
// on_http_get() - Responds to an HTTP GET request
//=========================================================================================================
void CHTTPServer::on_http_get(const char* resource)
{
    // Is this an HTTP get for "/"?
    if (strcmp(resource, "/") == 0)
    {
        reply_to_index();
        return;
    }

    // If we get here, the client was looking for an unknown webpage
    reply(404, "");
}
//=========================================================================================================



//=========================================================================================================
// on_http_post() - Responds to an HTTP POST request
//=========================================================================================================
void CHTTPServer::on_http_post(const char* resource)
{
    // Is this an HTTP POST for "/"?
    if (strcmp(resource, "/") == 0)
    {
        reply_to_index();
        return;
    }

    // Is this an "HTTP POST /reboot" ?
    if (strcmp(resource, "/reboot") == 0)
    {
        reply_to_index();
        msdelay(2000);
        System.reboot();
        return;
    }

    // Is this an "HTTP POST /brighter" ?
    if (strcmp(resource, "/brighter") == 0)
    {
        reply(201, "");
        printf(">> Brighter! <<\n");
        return;
    }


    // Is this an "HTTP POST /dimmer" ?
    if (strcmp(resource, "/dimmer") == 0)
    {
        reply(201, "");
        printf(">> Dimmer! <<\n");
        return;
    }

    // Is this "HTTP POST /config" ?
    if (strcmp(resource, "/config") == 0)
    {
        reply_to_config();
        return;
    }

    // Is this "HTTP POST /updatecfg" ?
    if (strcmp(resource, "/updatecfg") == 0)
    {
        save_updated_config();
        reply(200, "");
        return;
    }



    // If we get here, the client was looking for an unknown webpage
    reply(404, "");
}
//=========================================================================================================


//=========================================================================================================
// fetch_post_value() - Fetches the value portion of a "key=value" pair from the POST data
//=========================================================================================================
static bool fetch_post_value(const char* input, const char* key, char* output)
{
    // This is the token we're going to look for
    char token[30];

    // Ensure that the caller's output field starts out empty
    *output = 0;

    // Build the token ";key="
    token[0] = ';';
    strcpy(token+1, key);
    strcat(token, "=");

    printf("Searching for \"%s\"\n", token);

    // Does our token exist in the input string?
    const char* in = strstr(input, token);

    // If it doesn't, we're done here
    if (in == nullptr) return false;

    // Position ourselves immediately after the equals-sign
    in = strchr(in, '=') + 1;

    // Copy the requested value into the caller's buffer
    while (*in && *in!=';') *output++ = *in++;

    // Make sure the caller's buffer is nul-terminated
    *output = 0;

    // Tell the caller that we found their key and handed them the value they were looking for
    return true;
}
//=========================================================================================================




//=========================================================================================================
// save_updated_config() - Saves the updated configuration data
//=========================================================================================================
void CHTTPServer::save_updated_config()
{
    char buffer[128];

    // Fetch the network SSID
    if (fetch_post_value(m_request_content, "netssid", buffer))
    {
        safe_copy(NVS.data.network_ssid, buffer);
    }

    // Fetch the network password
    if (fetch_post_value(m_request_content, "netpw", buffer))
    {
        safe_copy(NVS.data.network_pw, buffer);
    }

    // Fetch the local offset (in hours) from UTC
    if (fetch_post_value(m_request_content, "utc_offset", buffer))
    {
        NVS.data.utc_offset = atoi(buffer);
    }

    // Commit these values to non-volatile storage
    NVS.write_to_flash();
}
//=========================================================================================================
