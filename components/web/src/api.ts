import type { ShallowRef } from "vue";
import type { Ref } from "vue";
import { shallowRef } from "vue";
import { ref } from "vue";

export enum ApiStatus {
    DISCONNECTED, CONNECTED, CONNECTIING
}

class Api {
    private ws: WebSocket
    public status: Ref<ApiStatus> = ref(ApiStatus.DISCONNECTED)
    //public actuator: Array<Ref<boolean>> = [...new Array(8)].map( () => ref(null))
    public actuator: ShallowRef<Array<boolean>> = shallowRef([])
    public sensors: Array<Ref<number>> = []
    public actuatorPendingUpdate = ref(true)
    public isLogin = ref(false)

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
        if (msg[0] == 0x01) {
            this.wsActuatorUpdate(msg[1])
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
