
export enum ApiStatus {
    DISCONNECTED, CONNECTED, CONNECTIING
}

/** #define WS_ON_OPEN 0x00 */
export const WS_ON_OPEN = 0x00
/** #define WS_MSG_ID_ACTUATOR 0x01 */
export const WS_MSG_ID_ACTUATOR = 0x01
/** #define WS_MSG_ID_CONFIG 0x02 */
export const WS_MSG_ID_CONFIG = 0x02
/** #define WS_MSG_ID_SENSOR 0x03 */
export const WS_MSG_ID_SENSOR = 0x03
export const WS_MSG_ID_LOGIN = 0x04
export const WS_MSG_ID_LOGOUT = 0x05
export const WS_MSG_ID_MESSAGE = 0x06
export const WS_MSG_ID_UPDATE_HOSTNAME = 0x07
export const WS_MSG_ID_UPDATE_SWITCHES = 0x08

export const WS_MSG_ID_STATE = 0x09
export const WS_MSG_ID_WIFI_STA_SCAN = 0x0a
export const WS_MSG_ID_WIFI_STA_UPDATE = 0x0b
export const WS_MSG_ID_WIFI_AP_UPDATE = 0x0c

export const TCPIP_HOSTNAME_MAX_SIZE = 32
export const APP_NAME_MAX_SIZE = 28;
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
    /** char[32]  */
    hostname: string
    /** char[28] unused in client side */
    password: string
    /** char[][28] Switch name */
    switches: string[]
    /** char[][28] Sensor name */
    sensors: string[]
}
