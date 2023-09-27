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

#define TAG "cmd_wifi"

static bool wifi_join(const char *ssid, const char *pass, int timeout_ms)
{
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (pass)
    {
        strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    int bits = xEventGroupWaitBits(state, STATE_STA_CONNECTED,
                                   1, 1, timeout_ms / portTICK_PERIOD_MS);
    return (bits & STATE_STA_CONNECTED) != 0;
}

static void print_interface_info(wifi_interface_t type)
{

    wifi_config_t wifi_config = {0};
    uint8_t mac[6];
    tcpip_adapter_ip_info_t ip_info;

    uint8_t *ssid, *password;

    const char *sUnknown = "(!) Unknown";
    const esp_mac_type_t mac_type = type == WIFI_IF_STA ? ESP_MAC_WIFI_STA : ESP_MAC_WIFI_SOFTAP;
    const tcpip_adapter_if_t tcpip_type = type == WIFI_IF_STA ? TCPIP_ADAPTER_IF_STA : TCPIP_ADAPTER_IF_AP;

    printf("\nInterface %s: ", type == WIFI_IF_STA ? "STA" : "AP");

    if (esp_wifi_get_config(type, &wifi_config) == ESP_OK)
    {
        EventBits_t bits = xEventGroupGetBits(state);
        int ssidLen = 0;
        const char *ifState;
        if (type == WIFI_IF_STA)
        {
            ssidLen = strlen((const char *)wifi_config.sta.ssid);
            ssid = wifi_config.sta.ssid;
            password = wifi_config.sta.password;
            if ((bits & STATE_STA_CONNECTING) == STATE_STA_CONNECTING)
            {
                ifState = "CONNECTING";
            }
            else if ((bits & STATE_STA_CONNECTED) == STATE_STA_CONNECTED)
            {
                ifState = "CONNECTED";
            }
            else if ((bits & STATE_STA_FAIL) == STATE_STA_FAIL)
            {
                ifState = "FAILED";
            }
            else
            {
                ifState = "UNKNWON";
            }
        }
        else
        {
            ssidLen = wifi_config.ap.ssid_len;
            ssid = wifi_config.ap.ssid;
            password = wifi_config.ap.password;
            if ((bits & STATE_AP_STARTED) == STATE_AP_STARTED)
            {
                ifState = "STARTED";
            }
            else
            {
                ifState = "DISABLED";
            }
        }
        printf("%s\n", ifState);
        if (ssidLen)
        {
            printf("\tSSID: %s\n\tPassword: %s\n", ssid, password);
        }
        else
        {
            printf("\t(!) Wifi Unconfigured\n");
        }
    }
    else
    {
        printf(sUnknown);
        printf("\n");
    }

    printf("\tMac: ");
    if (esp_read_mac(mac, mac_type) == ESP_OK)
    {
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        printf(sUnknown);
    }
    printf("\tIP : ");
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
            printf("null mode");
            break;
        case WIFI_MODE_STA:
            printf("WiFi station mode");
            break;
        case WIFI_MODE_AP:
            printf("WiFi soft-AP mode");
            break;
        case WIFI_MODE_APSTA:
            printf("WiFi station + soft-AP mode");
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

/** Arguments used by 'join' function */
static struct
{
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} join_args;

static int connect(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&join_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, join_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "Connecting to '%s'",
             join_args.ssid->sval[0]);

    bool connected = wifi_join(join_args.ssid->sval[0],
                               join_args.password->sval[0],
                               join_args.timeout->ival[0]);
    if (!connected)
    {
        ESP_LOGW(TAG, "Connection failed");
        return 1;
    }
    ESP_LOGI(TAG, "Connected");
    return 0;
}

void register_wifi()
{
    join_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    join_args.timeout->ival[0] = 5000; // set default value
    join_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    join_args.end = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command = "join",
        .help = "Join WiFi AP as a station",
        .hint = NULL,
        .func = &connect,
        .argtable = &join_args};

    ESP_ERROR_CHECK(esp_console_cmd_register(&join_cmd));
    const esp_console_cmd_t ip_cmd = {
        .command = "ip",
        .help = "IP and interface info",
        .hint = NULL,
        .func = &cmd_ip,
        .argtable = NULL};

    ESP_ERROR_CHECK(esp_console_cmd_register(&ip_cmd));
}
