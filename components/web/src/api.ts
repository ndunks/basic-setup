import type { Ref } from "vue";
import { shallowRef } from "vue";
import { ref } from "vue";
import { ApiStatus } from "./types";
import { bitsArraytoByte, bitsOnToArray, parseAppConfigStruct } from "./utils";
import { computed } from "vue";
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

    // Actuator option: 1,2,3,4,5,6,7,8
    public switchNames = shallowRef([] as string[]);
    public switchActives = shallowRef([] as number[])
    // public sensorNames = shallowRef([] as string[]);
    // public sensorValues = shallowRef([] as number[]);
    public hostname = shallowRef("...");

    private ws: WebSocket
    //private appConfig = shallowRef<AppConfig | null>(null)
    private autoReconnect = true
    /** Delay second before trying to reconnect */
    private autoReconnectBackoff = 2
    private waitingReplyTimeout = 5

    private waitingReplyStack: { code: number, callback: (msg: Uint8Array) => void }[] = []

    constructor(private serverURL: string) {
        this.connect()
        watch(() => this.waitingReplyStack.length, (v) => {
            this.isLoading.value = !!v
            console.debug(`Waiting stack`, v)
        })
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

    // Index of active switch
    updateActuator = (values: number[]) => {
        const byte = bitsArraytoByte(values)
        this.actuatorPendingUpdate.value = byte ^ bitsArraytoByte(this.switchActives.value)

        const uint8Array = new Uint8Array([0x01, byte])
        console.debug('REQ ACTUATOR UPDATE', values)
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

    updateSwitchNames(names: string[]) {
        const invalids = names.filter(v => v.length > APP_NAME_MAX_SIZE)

        if (invalids.length)
            return Promise.reject(`Max name length is ${APP_NAME_MAX_SIZE}`)

        return this.requestTruthy(WS_MSG_ID_UPDATE_SWITCHES, names.map(
            v => v.padEnd(APP_NAME_MAX_SIZE, "\x00")
        ).join(""))
    }

    logout() {
        this.send(WS_MSG_ID_LOGOUT)
        this.isLogin.value = false
    }

    send(code: number, payload?: string) {
        const len = payload ? payload.length : 0
        const v = new Uint8Array(len + 1);
        v[0] = code
        if (len > 0)
            v.set(payload.split('').map((v) => v.charCodeAt(0)), 1)
        this.ws.send(v)
        //this.isLogin.value = false
    }

    // Throw error on false
    requestTruthy(code: number, payload: string, errMsg = "Failed") {
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
    request(code: number, payload?: string) {
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
        }
        this.hostname.value = config.hostname
        this.switchNames.value = config.switches
        this.switchActives.value = bitsOnToArray(config.switchValues)

        this.isConnected.value = true
        this.autoReconnectBackoff = 2
        if(!this.isLogin.value){
            this.autoLogin()
        }
    }

    private wsActuatorUpdate(byte: number) {
        this.switchActives.value = bitsOnToArray(byte)
        this.actuatorPendingUpdate.value = 0
        console.debug('WS ACTUATOR UPDATE', this.switchActives.value)
    }

    private onGotBinaryMessage(msg: Uint8Array) {
        const code = msg[0]
        // Any waiters ?
        this.waitingReplyStack.filter(v => v.code === code).forEach(
            v => v.callback.call(this, msg.slice(1))
        )

        switch (code) {
            case WS_MSG_ID_ACTUATOR:
                return this.wsActuatorUpdate(msg[1])
            case WS_MSG_ID_CONFIG:
                return this.updateAppConfig(msg.slice(1))
        }
    }

    private wsOnOpen = () => {
        console.log("WS: Open");
    }

    private wsOnClose = () => {
        console.log("WS: Close");
        this.isConnected.value = false
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

globalThis.api = new Api(`ws://${wsHost}/ws`)
