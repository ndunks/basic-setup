#ifndef _WEB_SERVER_H
#define _WEB_SERVER_H

#include <esp_err.h>

/** Embeded binary of web files */
extern const char web_bin_start[] asm("_binary_web_bin_start");
extern const char web_bin_end[] asm("_binary_web_bin_end");

void web_server_main();

typedef struct
{
    const char *const name;
    const char *type;
    bool gzip;
    uint32_t offset;
    uint32_t size;
} webfs_t;

#ifdef CONFIG_LWIP_IPV6
    #define CLIENT_INFO_STR "[0000:0000:0000:0000:0000:0000:0000:0000]:PORT "
#else
    #define CLIENT_INFO_STR "000.000.000.000:PORT "
#endif
#define CLIENT_INFO_STR_LEN sizeof(CLIENT_INFO_STR)

// Generate command: node web.js
/* GENERATED CODE START */
static const char web_mime_image_png[] = "image/png";
static const char web_mime_application_javascript[] = "application/javascript";
static const char web_mime_text_css[] = "text/css";
static const char web_mime_text_html[] = "text/html";

static webfs_t const web_files[] = {
    {.name = "favicon.png", .type = web_mime_image_png, .gzip = false, .offset = 0, .size = 772},
    {.name = "index-5c5f0168.js", .type = web_mime_application_javascript, .gzip = true, .offset = 772, .size = 96697},
    {.name = "index-ed2142d7.css", .type = web_mime_text_css, .gzip = true, .offset = 97469, .size = 40536},
    {.name = "index.html", .type = web_mime_text_html, .gzip = true, .offset = 138005, .size = 282}};

#define INDEX_HTML_OFS 3
#define WEB_FILE_NAME_MAX 18
    // Total size: 138287 bytes
    // Generated at 11/4/2023, 9:25:56 PM
/* GENERATED CODE END */
#endif