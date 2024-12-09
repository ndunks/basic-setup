import type { Ref } from "vue";
import { shallowRef } from "vue";
import { ref } from "vue";
import { ApiStatus, SensorType, SwitchType } from "./types";
import { bitsFilterArraytoByte, bitsFilterOn, parseAppConfigStruct } from "./utils";
import { watch } from "vue";
import { reactive } from "vue";

export class Api {
    public status: Ref<ApiStatus> = ref(ApiStatus.DISCONNECTED)
    //public actuator: Array<Ref<boolean>> = [...new Array(8)].map( () => ref(null))
    public actuatorPendingUpdate = ref(0)
    public isConnected = ref(false)
    public isLogin = ref(false)
    public isLoading = ref(false)

    /** Device state */
    public state = reactive({
        /** BIT 1: AP mode started */
        AP_STARTED: false,
        /** BIT 2: Wifi in error state */
        WIFI_ERROR: false,
        /** BIT 3: Connected to the internet */
        INTERNET_CONNECTED: false,
        /** BIT 4: Trying to connect AP */
        STA_CONNECTING: false,
        /** BIT 5: Connected to AP */
        STA_CONNECTED: false,
        /** BIT 6: Fail to connect after several retry */
        STA_FAIL: false,
    })

    public switch = shallowRef({
        names: [] as string[],
        // Enable / disable
        status: [] as boolean[],
        types: [] as SwitchType[]
    })
    public sensor = shallowRef({
        names: [] as string[],
        // Enable / disable
        status: [] as boolean[],
        types: [] as SensorType[]
    })
    public switchActives = shallowRef([] as (number | null)[])
    public sensorValues = shallowRef([] as (number | null)[])
    public hostname = shallowRef("Loading..");

    private ws: WebSocket
    //private appConfig = shallowRef<AppConfig | null>(null)
    private autoReconnect = true
    /** Delay second before trying to reconnect */
    private autoReconnectBackoff = 2
    private waitingReplyTimeout = 5

    private waitingReplyStack: { code: number, callback: (msg: Uint8Array) => void }[] = []

    constructor(public host: string) {
        watch(() => this.waitingReplyStack.length, (v) => {
            this.isLoading.value = !!v
            console.debug(`Waiting stack`, v)
        })

        watch(this.status, status => {
            this.isConnected.value = status == ApiStatus.CONNECTED
        })

        this.connect()
    }

    connect() {
        this.autoReconnect = true
        this.status.value = ApiStatus.CONNECTIING

        if (this.ws) {
            throw new Error('Already connected')
        }

        this.ws = new WebSocket(`ws://${this.host}/ws`)
        this.ws.addEventListener('open', this.wsOnOpen);
        this.ws.addEventListener('close', this.wsOnClose);
        this.ws.addEventListener('message', this.wsOnMessage);
    }

    disconnect() {
        console.debug('WS Disconnect')
        this.status.value = ApiStatus.DISCONNECTED
        this.autoReconnect = false
        this.ws.close()
    }

    // Index of active switch
    updateActuators = (values: number[]) => {
        const byte = bitsFilterArraytoByte(values)
        this.actuatorPendingUpdate.value = byte ^ bitsFilterArraytoByte(this.switchActives.value)
        console.log(values, 'Pending', this.actuatorPendingUpdate.value)
        if (!sdkconfig.CONFIG_APP_ESP01_SUPPORT_LC) {
            const uint8Array = new Uint8Array([0x01, byte])
            console.debug('REQ ACTUATOR UPDATE MULTI', values)
            this.ws.send(uint8Array);
        }

    }

    updateSingleActuator(id: number, valueBool: boolean, toggle: Function) {

        toggle()

        const value = valueBool ? 1 : 0

        if (!sdkconfig.CONFIG_APP_ESP01_SUPPORT_LC) return;

        const byte = id << 4 | value & 0b1111
        const uint8Array = new Uint8Array([0x01, byte])
        console.debug('REQ ACTUATOR UPDATE SINGLE', byte, byte.toString(2))
        this.ws.send(uint8Array);
    }

    login(password: string, remember = false): Promise<boolean> {
        return this.requestTruthy(WS_MSG_ID_LOGIN, password, "Invalid password").then(
            res => {
                if (res && remember) {
                    localStorage.setItem("remember", password)
                }
                return this.isLogin.value = res
            }
        )
    }

    autoLogin() {
        const remembered = localStorage.getItem("remember")
        if (remembered) {
            return this.login(remembered).catch(
                e => false
            )
        }
        return Promise.resolve(false)
    }

    updateSwitchSensorConfig(items: { name: string, type: number, status: boolean }[], msgId: number = WS_MSG_ID_UPDATE_SWITCHES) {
        const invalids = items.filter(v => v.name.length > APP_NAME_MAX_SIZE)

        if (invalids.length)
            return Promise.reject(`Max name length is ${APP_NAME_MAX_SIZE}`)
        let x = []
        const req = new Uint8Array(
            [...items.flatMap(({ name }) => {
                return name.padEnd(APP_NAME_MAX_SIZE, "\x00").split('').map(s => s.charCodeAt(0))
            }),
            ...items.map(({ status, type }) => ((status ? 1 : 0) << 7) | type & 0x7f)
            ]
        )

        return this.requestTruthy(msgId, req)
    }

    logout() {
        this.send(WS_MSG_ID_LOGOUT)
        this.isLogin.value = false
    }

    send(code: number, payload?: string | Uint8Array) {
        const len = payload ? payload.length : 0
        const v = new Uint8Array(len + 1);
        v[0] = code
        if (len > 0) {
            if (typeof payload == 'string') {
                v.set(payload.split('').map((v) => v.charCodeAt(0)), 1)
            } else {
                v.set(payload, 1)
            }
        }
        this.ws.send(v)
        //this.isLogin.value = false
    }

    // Throw error on false
    requestTruthy(code: number, payload: string | Uint8Array, errMsg = "Failed") {
        return this.request(code, payload).then(
            res => {
                if (res[0] === 1) {
                    return true
                }
                return Promise.reject(new Error(errMsg))
            }
        )
    }

    /** Syncronus request-response */
    request(code: number, payload?: string | Uint8Array) {
        return new Promise<Uint8Array>((resolve, reject) => {
            let timeoutTimer
            const waiter = {
                code,
                callback(msg: Uint8Array) {
                    clearTimeout(timeoutTimer)
                    removeWaiter()
                    // maybe error message
                    if (msg[0] == WS_MSG_ID_MESSAGE) {
                        const errMsg = [...msg.slice(1)].map(v => String.fromCharCode(v)).join('')
                        reject(new Error(errMsg || 'Unknown error'))
                    } else {
                        resolve(msg)
                    }
                }
            }

            this.waitingReplyStack.push(waiter)
            const removeWaiter = () => {
                const i = this.waitingReplyStack.indexOf(waiter)
                // warn: race when modify array but already modified by other
                if (i >= 0)
                    this.waitingReplyStack.splice(i, 1)
            }


            timeoutTimer = setTimeout(() => {
                removeWaiter()
                reject(new Error('Timeout'))
            }, this.waitingReplyTimeout * 1000)
            try {
                this.send(code, payload)
            } catch (error) {
                clearTimeout(timeoutTimer)
                removeWaiter()
                reject(error)
            }
        })
    }

    private updateAppConfig(rawConfig: Uint8Array) {
        const config = parseAppConfigStruct(rawConfig);
        if (this.status.value == ApiStatus.CONNECTIING) {
            // First time get config
            this.status.value = ApiStatus.CONNECTED
            console.debug('WS Connected')
        }
        this.hostname.value = config.hostname
        this.switch.value = {
            names: config.switches,
            status: config.switchCfg.map(v => v.status),
            types: config.switchCfg.map(v => v.type),
        }
        this.switchActives.value = bitsFilterOn(config.switchValues)

        this.sensor.value = {
            names: config.sensors,
            status: config.sensorCfg.map(v => v.status),
            types: config.sensorCfg.map(v => v.type),
        }
        this.sensorValues.value = config.sensors.map(() => null)
        this.autoReconnectBackoff = 2

        if (!this.isLogin.value) {
            this.autoLogin()
        }
    }

    private wsOnSwitchUpdate(byte: number) {
        this.switchActives.value = bitsFilterOn(byte)
        this.actuatorPendingUpdate.value = 0
        console.debug('WS ACTUATOR UPDATE', this.switchActives.value)
    }

    private wsOnSensorUpdate(b: Uint8Array) {
        const len = b[0]
        const values: number[] = []
        let v = 0
        for (let i = 0; i < len; i++) {
            v = b[i + 1] << 0 >>> 0
            v = Math.round(v / 2.55)
            values.push(v)
        }
        this.sensorValues.value = values
    }

    private onGotBinaryMessage(msg: Uint8Array) {
        const code = msg[0]
        // Any waiters ?
        this.waitingReplyStack.filter(v => v.code === code).forEach(
            v => v.callback.call(this, msg.slice(1))
        )

        switch (code) {
            case WS_MSG_ID_ACTUATOR:
                return this.wsOnSwitchUpdate(msg[1])
            case WS_MSG_ID_SENSOR:
                return this.wsOnSensorUpdate(msg.slice(1))
            case WS_MSG_ID_CONFIG:
                return this.updateAppConfig(msg.slice(1))
        }
    }

    private wsOnOpen = () => {
        console.log("WS: Open");
    }

    private wsOnClose = () => {
        console.log("WS: Close");
        this.status.value = ApiStatus.DISCONNECTED
        this.isLogin.value = false

        this.ws.removeEventListener('open', this.wsOnOpen);
        this.ws.removeEventListener('close', this.wsOnClose);
        this.ws.removeEventListener('message', this.wsOnMessage);
        this.ws = null

        if (this.autoReconnect) {
            setTimeout(() => {
                if (this.autoReconnectBackoff < 10)
                    this.autoReconnectBackoff++

                this.connect()
            }, this.autoReconnectBackoff * 1000)
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

globalThis.api = new Api(wsHost)
