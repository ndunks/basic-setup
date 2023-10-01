/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "web.h"

#include <esp_http_server.h>

static const char *TAG = "web";

/* An HTTP GET handler */
static esp_err_t get_handler(httpd_req_t *req)
{
    const webfs_t *f = &web_files[INDEX_HTML_OFS];
    // char bufs[64];

    // todo: matching with URL here

    httpd_resp_set_type(req, f->type);
    // sprintf(bufs, "%d", f->size);
    // httpd_resp_set_hdr(req, "XContent-Length", bufs);
    if (f->gzip)
    {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(
        httpd_resp_send(req, web_bin_start + f->offset, f->size));
    // httpd_resp_send(req, "HELO", 4);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    // if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    // {
    //     ESP_LOGI(TAG, "Request headers lost");
    // }
    return ESP_OK;
}

httpd_handle_t start_webserver(void)
{
    httpd_uri_t default_handler = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_handler,
        .user_ctx = NULL};

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        ESP_ERROR_CHECK_WITHOUT_ABORT(httpd_register_uri_handler(server, &default_handler));
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static httpd_handle_t server = NULL;

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void web_main()
{
    // ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    // ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    server = start_webserver();
}