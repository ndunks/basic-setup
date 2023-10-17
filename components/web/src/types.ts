
export enum ApiStatus {
    DISCONNECTED, CONNECTED, CONNECTIING
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
    /** char[32]  */
    hostname: string
    /** char[28] unused in client side */
    password: string
    /** char[][28] Switch name */
    switches: string[]
    /** char[][28] Sensor name */
    sensors: string[]
}
