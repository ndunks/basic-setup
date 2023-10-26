#include <string.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <errno.h>
#include <esp_log.h>
#include <esp_err.h>
#include "esp_event.h"

#include "web-server.h"
#include "web-socket.h"

#define TAG "WEBSERVER"
#define HTTP_BUF_SIZE 2048
#define HTTP_LISTEN_PORT 80
/** how many incoming connections can queue up if your application isn't accept()ing */
#define HTTP_LISTEN_BACKLOG 2
#define HTTP_STATUS_200_OK 200
#define HTTP_STATUS_404_NOT_FOUND 404
#define HTTP_STATUS_400_BAD_REQUEST 400
#define HTTP_STATUS_501_NOT_IMPLEMENTED 501
#define HTTP_STATUS_429_TO_MANY 429
#define WEBSOCKET_HANDLER -0x400

/**
 * Usefull links
 * https://www.freertos.org/FreeRTOS_Support_Forum_Archive/November_2017/freertos_FreeRTOS_TCP_and_multi-threading_72bb9778j.html
 * https://github.com/particle-iot/freertos/blob/master/FreeRTOS-Plus/Demo/Common/Demo_IP_Protocols/Common/FreeRTOS_TCP_server.c#L163
 */

static int fd = -1, client_fd = -1;
static TaskHandle_t http_task_handle;
static SemaphoreHandle_t xSemaphore;
static char http_buf[HTTP_BUF_SIZE + 1] = {0};
static char client_info[CLIENT_INFO_STR_LEN] = {0};
/**
 * Handle HTTP request.
 * Responsible to close the fd if is not UPGRADE/WebSocket request.
 */

static esp_err_t http_response(const char *type, int size, const char *body, int code, bool is_gzip, bool cache)
{
    const char *resp = "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: Close\r\n";
    const char *status;
    const char gzip_header[] = "Content-Encoding: gzip\r\n";
    const char cache_header[] = "Cache-Control: public, max-age=86400\r\n";
    switch (code)
    {
    case HTTP_STATUS_200_OK:
        status = "200 OK";
        break;
    case HTTP_STATUS_429_TO_MANY:
        status = "429 Too Many Requests";
        break;
    case HTTP_STATUS_400_BAD_REQUEST:
        status = "400 Bad Request";
        break;
    case HTTP_STATUS_501_NOT_IMPLEMENTED:
        status = "501 Not Implemented";
        break;
    default:
        status = "500 Server Error";
        break;
    }
    // write header
    int len = sprintf(http_buf, resp, status, type, size);
    write(client_fd, http_buf, len);
    if (is_gzip)
    {
        write(client_fd, gzip_header, sizeof(gzip_header) - 1);
    }
    if (cache)
    {
        write(client_fd, cache_header, sizeof(cache_header) - 1);
    }
    write(client_fd, "\r\n", 2);

    // write body
    if (size > 0 && body != NULL)
    {
        write(client_fd, body, size);
    }

    return ESP_OK;
}

static esp_err_t http_handler()
{
    ssize_t http_header_len;
    int len, i;

    // if (ioctlsocket(client_fd, FIONREAD, &len))
    // {
    //     ESP_LOGI(TAG, "Req sz %u", len);
    // }

    http_header_len = recv(client_fd, http_buf, HTTP_BUF_SIZE, 0);
    http_buf[http_header_len] = 0;
    ESP_LOGI(TAG, "%u bytes\n%s", http_header_len, http_buf);

    // Minimal expected content is:
    // GET / HTTP/1.1\r\n\r\n (18 byte)
    if (http_header_len < 18)
    {
        http_response(web_mime_text_html, 0, NULL, HTTP_STATUS_400_BAD_REQUEST, false, false);
        goto jmp_end;
    }

    // validating http request header
    if (strncmp(&http_buf[http_header_len - 4], "\r\n\r\n", 4) != 0)
    {
        http_response(web_mime_text_html, 0, NULL, HTTP_STATUS_400_BAD_REQUEST, false, false);
        goto jmp_end;
    }

    if (strncmp(http_buf, "GET", 3) != 0)
    {
        http_response(web_mime_text_html, 0, NULL, HTTP_STATUS_501_NOT_IMPLEMENTED, false, false);
        goto jmp_end;
    }

    char *path = http_buf + 5; // remove: GET /
    char *endPath = strchr(path, ' ');
    // Default to index.html, its SPA web
    const webfs_t *f = &web_files[INDEX_HTML_OFS];

    if (!endPath)
    {
        http_response(web_mime_text_html, 0, NULL, HTTP_STATUS_400_BAD_REQUEST, false, false);
        goto jmp_end;
    }
    // replace space with null
    //*endPath = 0;
    int path_len = endPath - path;

    // ESP_LOGI(TAG, "Path (%d) %s", path_len, path);

    if (path_len > 1 && path_len <= WEB_FILE_NAME_MAX)
    {
        // Check for wss
        if (strncmp(path, "ws", 2) == 0)
        {
            i = ws_accept(client_fd, client_info, http_buf, http_header_len);

            if (i == ESP_OK)
                return WEBSOCKET_HANDLER;

            if (i == WS_ERR_TOO_MANY_CLIENT)
            {
                http_response(web_mime_text_html, 0, NULL, HTTP_STATUS_429_TO_MANY, false, false);
                goto jmp_end;
            }
            // just return, closed in web-socket
            return ESP_OK;
        }
        for (i = 0; i < ((sizeof web_files) / sizeof(webfs_t)); i++)
        {
            const webfs_t *tmpf = &web_files[i];
            if (strncmp(tmpf->name, path, path_len) == 0)
            {
                f = tmpf;
            }
        }
    }

    http_response(f->type, f->size, &web_bin_start[f->offset], HTTP_STATUS_200_OK, f->gzip, true);
jmp_end:
    // Close the FD
    close(client_fd);
    return ESP_OK;
}

/**
 * HTTP thread loop
 * Only 1 task to handle HTTP req a time
 */
static void http_thread(void *arg)
{
    struct sockaddr_in addr_from;
    socklen_t addr_from_len = sizeof(addr_from);

    struct timeval tv = {
        .tv_sec = 5,
        .tv_usec = 0,
    };

    ESP_LOGD(TAG, "started");
    while (1)
    {
        // This is blocking
        client_fd = accept(fd, (struct sockaddr *)&addr_from, &addr_from_len);
        if (client_fd < 0)
        {
            ESP_LOGW(TAG, "error in accept (%d)", errno);
            break;
        }
#ifdef CONFIG_LWIP_IPV6
        sprintf(client_info, "[" IPV6STR "]:%u", IPV62STR((ip4_addr_t *)&addr_from.sin_addr), ntohs(addr_from.sin_port));
#else
        sprintf(client_info, IPSTR ":%u", IP2STR((ip4_addr_t *)&addr_from.sin_addr), ntohs(addr_from.sin_port));
#endif
        ESP_LOGI(client_info, "REQ FD %d", client_fd);
        // setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
        setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));

        if (http_handler() == ESP_OK)
            ESP_LOGI(client_info, "CLOSE");
    }
    ESP_LOGD(TAG, "exiting");
}

static bool started = false;

static void http_start(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (started)
        return;
    ESP_LOGI(TAG, "Starting webserver");
    xSemaphore = xSemaphoreCreateMutex();

    if (xSemaphore == NULL)
    {
        ESP_LOGE(TAG, "xSemaphoreCreateMutex (%d)", errno);
        return;
    }

#ifdef CONFIG_LWIP_IPV6
    fd = socket(PF_INET6, SOCK_STREAM, 0);
#else
    fd = socket(PF_INET, SOCK_STREAM, 0);
#endif /* CONFIG_LWIP_IPV6 */
    if (fd < 0)
    {
        ESP_LOGE(TAG, "error in socket (%d)", errno);
        return;
    }

#ifdef CONFIG_LWIP_IPV6
    struct in6_addr inaddr_any = IN6ADDR_ANY_INIT;
    struct sockaddr_in6 serv_addr = {
        .sin6_family = PF_INET6,
        .sin6_addr = inaddr_any,
        .sin6_port = htons(HTTP_LISTEN_PORT)};
#else
    struct sockaddr_in serv_addr = {
        .sin_family = PF_INET,
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY)},
        .sin_port = htons(HTTP_LISTEN_PORT)};
#endif /* CONFIG_LWIP_IPV6 */

    /* Enable SO_REUSEADDR to allow binding to the same
     * address and port when restarting the server */
    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
    {
        /* This will fail if CONFIG_LWIP_SO_REUSE is not enabled. But
         * it does not affect the normal working of the HTTP Server */
        ESP_LOGW(TAG, "error in setsockopt SO_REUSEADDR (%d)", errno);
    }

    int ret = bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        ESP_LOGE(TAG, "error in bind (%d)", errno);
        close(fd);
        return;
    }

    ret = listen(fd, HTTP_LISTEN_BACKLOG);
    if (ret < 0)
    {
        ESP_LOGE(TAG, "error in listen (%d)", errno);
        close(fd);
        return;
    }

    ret = xTaskCreate(&http_thread, "http", 4096, NULL, 5, &http_task_handle);

    if (ret == pdPASS)
    {
        return;
    }
    close(fd);
    return;
}

static void http_stop(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Stopping webserver");
    vTaskDelete(http_task_handle);
    close(fd);
    vSemaphoreDelete(xSemaphore);
    started = false;
}

void web_server_main()
{
    // Websocket setup
    web_socket_main();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &http_start, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &http_stop, NULL));
}