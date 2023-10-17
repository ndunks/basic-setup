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

// Generate command: node web.js
/* GENERATED CODE START */
static const char web_mime_image_png[] = "image/png";
static const char web_mime_application_javascript[] = "application/javascript";
static const char web_mime_text_css[] = "text/css";
static const char web_mime_text_html[] = "text/html";

static webfs_t const web_files[] = {
    {.name = "favicon.png", .type = web_mime_image_png, .gzip = false, .offset = 0, .size = 772},
    {.name = "index-4c5ae614.js", .type = web_mime_application_javascript, .gzip = true, .offset = 772, .size = 85590},
    {.name = "index-8617b0d9.css", .type = web_mime_text_css, .gzip = true, .offset = 86362, .size = 39184},
    {.name = "index.html", .type = web_mime_text_html, .gzip = true, .offset = 125546, .size = 283}};

#define INDEX_HTML_OFS 3
#define WEB_FILE_NAME_MAX 18
    // Total size: 125829 bytes
    // Generated at 10/17/2023, 12:19:21 PM
/* GENERATED CODE END */
#endif