
export enum ApiStatus {
    DISCONNECTED, CONNECTED, CONNECTIING
}
export enum SwitchType {
    SWITCH = 1, PUSH
}
export enum SensorType {
    BAR = 1, CIRCLE
}
export interface ApiItem {
    id: number
    kind: 'switch' | 'sensor'
    type: SwitchType | SensorType
    status: boolean
    name: string
    value: boolean | number
}

export interface ApiItemSwitch extends ApiItem {
    kind: 'switch'
    type: SwitchType
    /** ON/OFF */
    value: boolean
}

export interface ApiItemSensor extends ApiItem {
    kind: 'sensor'
    type: SensorType
    /** 0 to 100 */
    value: number
}

/**
struct app_config
{
    uint16_t config_version;
    uint8_t switch_len;
    uint8_t sensor_len;
    uint8_t switch_values; // APP_SWITCH_COUNT / 8
    char hostname[TCPIP_HOSTNAME_MAX_SIZE];
    char password[APP_NAME_MAX_SIZE];
    char switches[APP_SWITCH_COUNT][APP_NAME_MAX_SIZE];
    char sensors[APP_SENSOR_COUNT][APP_NAME_MAX_SIZE];
};
 */
export interface AppConfig {
    /** uint16_t */
    configVersion: number
    /** uint8_t */
    switchLen: number
    /** uint8_t */
    sensorLen: number
    /** uint8_t */
    switchValues: number
    /** uint8_t */
    switchCfg: { status: boolean, type: SwitchType }[]
    /** uint8_t */
    sensorCfg: { status: boolean, type: SensorType }[]
    /** uint8_t */
    reserved1: number
    /** uint16_t */
    sensor_delay: number
    /** uint16_t */
    reserved2: number
    /** char[32]  */
    hostname: string
    /** char[][28] Switch name */
    switches: string[]
    /** char[][28] Sensor name */
    sensors: string[]
}
/**
 * TCP-IP adatpter IPV4 
 */
export interface IpInfo {
    /** u32_t addr */
    ip: string
    /** u32_t addr */
    netmask: string
    /** u32_t addr */
    gw: string
}
export interface WifiConfigSta {
    isEnabled: boolean
    /** unsigned char mac[6] */
    mac: string;
    /** tcpip_adapter_ip_info_t net */
    net: IpInfo;
    /** char ssid[32] */
    ssid: string;
    /** char password[64] */
    password: string;
    autoConnect?: boolean
};
export interface WifiConfigAp {
    // unsigned char
    isEnabled: boolean
    /** unsigned char[6] */
    mac: string;
    /** tcpip_adapter_ip_info_t net */
    net: IpInfo;
    /** char[32] */
    ssid: string;
    /** char[64] */
    password: string;
    /** char **/
    isStarted: boolean
    /** char **/
    authmode: WifiAuthMode
}

export enum WifiMode {
    /** WIFI_MODE_NULL: null mode */
    Invactive,
    /** WIFI_MODE_STA: WiFi station mode */
    STA,
    /** WIFI_MODE_AP: WiFi soft-AP mode */
    AP,
    /** WIFI_MODE_APSTA: WiFi station + soft-AP mode */
    "AP+STA",
    /** WIFI_MODE_MAX */
    Unknown
}
//  struct ws_scan_result
export interface WifiScanResult {
    /** MAC address of AP */
    bssid: string
    /** char[33 SSID of AP */
    ssid: string
    /** signal strength of AP */
    rssi: number
    authmode: WifiAuthMode
}

export enum WifiAuthMode {
    /** WIFI_AUTH_OPEN */
    "open",
    /** WIFI_AUTH_WEP */
    "WEP",
    /** WIFI_AUTH_WPA_PSK */
    "WPA/PSK",
    /** WIFI_AUTH_WPA2_PSK */
    "WPA2/PSK",
    /** WIFI_AUTH_WPA_WPA2_PSK */
    "WPA/WPA2/PSK",
    /** WIFI_AUTH_WPA2_ENTERPRISE */
    "WPA2/ENTERPRISE",
    /** WIFI_AUTH_WPA3_PSK */
    "WPA3/PSK",
    /** WIFI_AUTH_WPA2_WPA3_PSK */
    "WPA2/WPA3/PSK",
    /** WIFI_AUTH_MAX */
    "Unknown"
}