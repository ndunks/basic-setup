import type { ShallowRef } from "vue";
import type { Ref } from "vue";
import { shallowRef } from "vue";
import { ref } from "vue";

export enum ApiStatus {
    DISCONNECTED, CONNECTED, CONNECTIING
}

/** #define WS_ON_OPEN 0x00 */
const WS_ON_OPEN = 0x00
/** #define WS_MSG_ID_ACTUATOR 0x01 */
const WS_MSG_ID_ACTUATOR = 0x01
/** #define WS_MSG_ID_CONFIG 0x02 */
const WS_MSG_ID_CONFIG = 0x02
/** #define WS_MSG_ID_SENSOR 0x03 */
const WS_MSG_ID_SENSOR = 0x03

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
interface AppConfig {
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
    /** char[28] */
    password: string
    /** char[][28] Switch name */
    switches: string[]
    /** char[][28] Sensor name */
    sensors: string[]
}

function parseAppConfigStruct(b: Uint8Array): AppConfig {
    const hostnameMaxLen = 32, strMaxLen = 28;

    let configVersion: number,
        switchLen: number,
        sensorLen: number,
        switchValues: number,
        hostname: string,
        password: string,
        switches: string[],
        sensors: string[];
        let ofs = 0;

        function readStr(maxLen: number){
            const str = [...b.slice(ofs, ofs + maxLen)]
            .filter( v => !!v).map( v => String.fromCharCode(v)).join('')
            ofs += maxLen
            return str
        }


        // From Bigendian, readUint16
        configVersion = (((b[ofs]) & 0xff) >>> 0) | ((b[ofs+1] << 8) & 0xff) >>> 0;
        ofs += 2
        switchLen = b[ofs++];
        sensorLen = b[ofs++];
        switchValues = b[ofs++];
        // if( configVersion != 1 ){
        //     console.warn('Unsupported config version', configVersion)
        //     return null;
        // }
        hostname = readStr(hostnameMaxLen)
        password = readStr(strMaxLen)
        switches = [...new Array(switchLen)].map( (_, i) => {
            return readStr(strMaxLen)
        })
        sensors = [...new Array(sensorLen)].map( (_, i) => {
            return readStr(strMaxLen)
        })

    const cfg: AppConfig = {
        configVersion,
        switchLen,
        sensorLen,
        switchValues,
        hostname,
        password,
        switches,
        sensors,
    }
    console.debug(cfg)
    return cfg
}

class Api {
    private ws: WebSocket
    public status: Ref<ApiStatus> = ref(ApiStatus.DISCONNECTED)
    //public actuator: Array<Ref<boolean>> = [...new Array(8)].map( () => ref(null))
    public actuator: ShallowRef<Array<boolean>> = shallowRef([])
    public sensors: Array<Ref<number>> = []
    public actuatorPendingUpdate = ref(true)
    public isLogin = ref(false)
    public appConfig = shallowRef<AppConfig | null>(null)

    // Actuator option: 1,2,3,4,5,6,7,8
    public actuatorOptions = [...new Array(8)].map((_, i) => (i + 1).toString())
    private autoReconnect = true

    constructor(private serverURL: string) {
        this.connect()
    }

    connect() {
        this.autoReconnect = true
        this.status.value = ApiStatus.CONNECTIING

        if (this.ws) {
            throw new Error('Already connected')
        }

        this.ws = new WebSocket(this.serverURL)
        this.ws.addEventListener('open', this.wsOnOpen);
        this.ws.addEventListener('close', this.wsOnClose);
        this.ws.addEventListener('message', this.wsOnMessage);
    }

    disconnect() {
        this.status.value = ApiStatus.DISCONNECTED
        this.autoReconnect = false
        this.ws.close()
    }

    updateActuator = (values: string[]) => {
        let value: boolean
        let actuatorName: string
        let byte = 0
        // push to websocket, map back as byte
        for (let i = 0; i < 8; i++) {
            actuatorName = (i + 1).toString()
            value = values.includes(actuatorName)
            if (value) {
                byte |= 1 << i
            }
        }

        this.actuatorPendingUpdate.value = true
        const uint8Array = new Uint8Array([0x01, byte])
        console.debug('REQ ACTUATOR UPDATE', values)
        this.ws.send(uint8Array);
    }

    private wsActuatorUpdate(byte: number) {
        let value: boolean
        const actuator = []
        for (let i = 0; i < 8; i++) {
            value = !!(byte & 1 << i)
            if (value) {
                actuator.push((i + 1).toString())
            }
        }
        this.actuator.value = actuator
        this.actuatorPendingUpdate.value = false
        console.debug('WS ACTUATOR UPDATE', actuator)
    }

    private onGotBinaryMessage(msg: Uint8Array) {
        // Actuator update
        switch (msg[0]) {
            case WS_MSG_ID_ACTUATOR:
                return this.wsActuatorUpdate(msg[1])
            case WS_MSG_ID_CONFIG:
                this.appConfig.value = parseAppConfigStruct(msg.slice(1));
                break
        }
    }

    private wsOnOpen = () => {
        console.log("WS: Open");
        this.status.value = ApiStatus.CONNECTED
    }

    private wsOnClose = () => {
        console.log("WS: Close");
        this.ws.removeEventListener('open', this.wsOnOpen);
        this.ws.removeEventListener('close', this.wsOnClose);
        this.ws.removeEventListener('message', this.wsOnMessage);
        this.ws = null
        if (this.autoReconnect) {
            setTimeout(() => {
                this.connect()
            }, 7000)
        }
    }

    private wsOnMessage = (event: MessageEvent<string | Blob>) => {
        if (typeof event.data == 'string') {
            console.log('Msg', event.data)
        } else { // Blob
            event.data.arrayBuffer().then(
                arrayBuffer => {
                    this.onGotBinaryMessage(new Uint8Array(arrayBuffer))
                }
            )
        }
    }
}

let wsHost = location.host
if (import.meta.env.DEV && import.meta.env.VITE_API) {
    wsHost = import.meta.env.VITE_API
}

const api = new Api(`ws://${wsHost}/ws`)

export function useApi() {
    return api
}
