#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

    extern const char *web_bin_start asm("_binary_web_bin_start");
    extern const char *web_bin_end asm("_binary_web_bin_end");

    void web_main(void);

    typedef struct webfs
    {
        const char *name;
        const char *type;
        bool gzip;
        uint32_t offset;
        uint32_t size;
    } webfs_t;

    // Generate command: node web.js
    /* GENERATED CODE START */

    static const char *const web_mimes[] = {
        "image/png",
        "application/javascript",
        "text/css",
        "text/html"};

    static const webfs_t web_files[] = {
        {.name = "favicon.png", .type = web_mimes[0], .gzip = false, .offset = 0, .size = 772},
        {.name = "index-09553b7f.js", .type = web_mimes[1], .gzip = true, .offset = 772, .size = 54175},
        {.name = "index-cbe7d8f5.css", .type = web_mimes[2], .gzip = true, .offset = 54947, .size = 32049},
        {.name = "index.html", .type = web_mimes[3], .gzip = true, .offset = 86996, .size = 667}};

#define INDEX_HTML_OFS 3
    // Total size: 87663 bytes
    // Generated at 10/1/2023, 7:40:26 PM
    /* GENERATED CODE END */
#ifdef __cplusplus
}
#endif