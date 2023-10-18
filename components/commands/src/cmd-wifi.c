#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "tcpip_adapter.h"
#include "esp_event_loop.h"
#include "commands.h"
#include "state.h"
#include "wifi.h"
#include "colors.h"

#define TAG "cmd_wifi"

static void print_interface_info(wifi_interface_t type)
{

    wifi_config_t wifi_config = {0};
    uint8_t mac[6];
    tcpip_adapter_ip_info_t ip_info;

    uint8_t *ssid, *password;

    const char *sUnknown = "(!) Unknown",
               *s_auto_connect = "Auto Connect: %s\n",
               *s_auth_mode = "Auth Mode: %s\n";

    char if_more[128] = {0};
    const esp_mac_type_t mac_type = type == WIFI_IF_STA ? ESP_MAC_WIFI_STA : ESP_MAC_WIFI_SOFTAP;
    const tcpip_adapter_if_t tcpip_type = type == WIFI_IF_STA ? TCPIP_ADAPTER_IF_STA : TCPIP_ADAPTER_IF_AP;

    printf("\n--- Interface: %s ---\n", type == WIFI_IF_STA ? "STA" : "AP");

    if (esp_wifi_get_config(type, &wifi_config) == ESP_OK)
    {
        EventBits_t bits = STATE();
        int ssidLen = 0;
        const char *ifState;
        if (type == WIFI_IF_STA)
        {
            ssidLen = strlen((const char *)wifi_config.sta.ssid);
            ssid = wifi_config.sta.ssid;
            password = wifi_config.sta.password;
            if (IS_BITS(STATE_STA_CONNECTING, bits))
                ifState = COLOR_INFO("CONNECTING");
            else if (IS_BITS(STATE_STA_CONNECTED, bits))
                ifState = COLOR_OK("CONNECTED");
            else if (IS_BITS(STATE_STA_FAIL, bits))
                ifState = COLOR_ERR("FAILED");
            else
                ifState = COLOR_WARN("DISCONNECTED");
            bool autoC;
            esp_wifi_get_auto_connect(&autoC);
            if (autoC)
                sprintf(if_more, s_auto_connect, COLOR_OK("Yes"));
            else
                sprintf(if_more, s_auto_connect, COLOR_ERR("No"));
        }
        else
        {
            ssidLen = wifi_config.ap.ssid_len;
            ssid = wifi_config.ap.ssid;
            password = wifi_config.ap.password;

            if ((bits & STATE_AP_STARTED) == STATE_AP_STARTED)
                ifState = COLOR_OK("STARTED");
            else
                ifState = COLOR_ERR("STOPPED");
            sprintf(if_more, s_auth_mode, wifi_authmode_names[wifi_config.ap.authmode]);    
        }
        printf("Status: %s\n", ifState);
        if (ssidLen)
        {
            printf("SSID: %s\nPassword: %s\n", ssid, password);
        }
        else
        {
            printf("(!) Wifi Unconfigured\n");
        }
        printf(if_more);
    }
    else
    {
        printf(sUnknown);
        printf("\n");
    }

    printf("Mac: ");
    if (esp_read_mac(mac, mac_type) == ESP_OK)
    {
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        printf(sUnknown);
    }
    printf("IP : ");
    if (tcpip_adapter_get_ip_info(tcpip_type, &ip_info) == ESP_OK)
    {
        int cidr = 0;
        u32_t netmask = ntohl(ip_info.netmask.addr);
        while (netmask)
        {
            cidr += (netmask & 0x01);
            netmask >>= 1;
        }

        // don't put ip4addr_ntoa in once call, it using static char buffer.
        printf("%s/%u", ip4addr_ntoa(&(ip_info.ip)), cidr);
        printf(" gateway %s", ip4addr_ntoa(&(ip_info.gw)));
    }
    else
    {
        printf(sUnknown);
    }
    printf("\n");
}

static int cmd_ip(int argc, char **argv)
{
    wifi_mode_t mode;

    if (esp_wifi_get_mode(&mode) == ESP_OK)
    {
        printf("Wifi Mode: ");
        switch (mode)
        {
        case WIFI_MODE_NULL:
            printf("NONE");
            break;
        case WIFI_MODE_STA:
            printf("STA");
            break;
        case WIFI_MODE_AP:
            printf("AP");
            break;
        case WIFI_MODE_APSTA:
            printf("STA + AP");
            break;

        default:
            printf("Unknown (%x)", mode);
            break;
        }
        printf("\n");
    }

    print_interface_info(WIFI_IF_STA);
    print_interface_info(WIFI_IF_AP);
    return 0;
}

/** Arguments used by 'connect' function */
static struct
{
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} connect_args;

static esp_err_t cmd_connect(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&connect_args);
    const char *ssid = NULL, *pass = NULL;

    if (nerrors != 0)
    {
        arg_print_errors(stderr, connect_args.end, argv[0]);
        return 1;
    }

    if (connect_args.ssid->count == 1)
    {
        ssid = connect_args.ssid->sval[0];
        if (connect_args.password->count == 1)
        {
            pass = connect_args.password->sval[0];
        }
    }

    return app_wifi_connect(ssid, pass);
}

static esp_err_t cmd_disconnect(int argc, char **argv)
{
    return app_wifi_disconnect();
}

static esp_err_t cmd_ap_start(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&connect_args);
    const char *ssid = NULL, *pass = NULL;

    if (nerrors != 0)
    {
        arg_print_errors(stderr, connect_args.end, argv[0]);
        return 1;
    }

    if (connect_args.ssid->count == 1)
    {
        ssid = connect_args.ssid->sval[0];
        if (connect_args.password->count == 1)
        {
            pass = connect_args.password->sval[0];
        }
    }

    return app_wifi_ap_start(ssid, pass);
}

static esp_err_t cmd_ap_stop(int argc, char **argv)
{
    return app_wifi_ap_stop();
}

static void print_wifi_scan_results(uint16_t *len, wifi_ap_record_t *records)
{
    for (int i = 0; i < *len; i++)
    {
        printf("%s\t rssi %d auth %s\n", records[i].ssid, records[i].rssi, wifi_authmode_names[records[i].authmode]);
    }
}

static esp_err_t cmd_ap_scan(int argc, char **argv)
{
    return app_wifi_scan(&print_wifi_scan_results);
}

void register_wifi()
{

    connect_args.ssid = arg_str0(NULL, NULL, "<ssid>", "SSID");
    connect_args.password = arg_str0(NULL, NULL, "<pass>", "Password");
    connect_args.end = arg_end(1);

    const esp_console_cmd_t ip_cmd = {
        .command = "ip",
        .help = "IP and interface info",
        .hint = NULL,
        .func = &cmd_ip,
        .argtable = NULL};

    const esp_console_cmd_t disconnect_cmd = {
        .command = "disconnect",
        .help = "Disconnect wifi STA from AP",
        .hint = NULL,
        .func = &cmd_disconnect,
        .argtable = NULL};

    const esp_console_cmd_t connect_cmd = {
        .command = "connect",
        .help = "Join WiFi AP as a station",
        .hint = NULL,
        .func = &cmd_connect,
        .argtable = &connect_args};

    const esp_console_cmd_t stop_cmd = {
        .command = "stop",
        .help = "Stop AP",
        .hint = NULL,
        .func = &cmd_ap_stop,
        .argtable = NULL};

    const esp_console_cmd_t start_cmd = {
        .command = "start",
        .help = "Start WiFi AP",
        .hint = NULL,
        .func = &cmd_ap_start,
        .argtable = &connect_args};

    const esp_console_cmd_t scan_cmd = {
        .command = "scan",
        .help = "Scan WiFi",
        .hint = NULL,
        .func = &cmd_ap_scan,
        .argtable = NULL};

    ESP_ERROR_CHECK(esp_console_cmd_register(&connect_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&disconnect_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&start_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&stop_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&scan_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&ip_cmd));
}
