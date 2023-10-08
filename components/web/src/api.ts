import type { Ref } from "vue";
import { ref } from "vue";
import { reactive } from "vue";

let socket: WebSocket;
let echoTest: number

let cbMsg: (msg: string | Blob) => void

export class Api {
    private ws: WebSocket

    public actuator: Array<Ref<boolean>> = []
    public sensors: Array<Ref<number>> = []

    constructor(private serverURL: string) {

    }

    connect() {
        if (this.ws) {
            throw new Error('Already connected')
        }

        this.ws = new WebSocket(this.serverURL)
        this.ws.addEventListener('open', this.wsOnOpen);
        this.ws.addEventListener('close', this.wsOnClose);
        this.ws.addEventListener('message', this.wsOnMessage);
    }

    private actuatorUpdate(byte: number) {
        let value: boolean
        for (let i = 0; i < 8; i++) {
            value = !!(byte & 1 << i)
            if (typeof this.actuator[i] === 'undefined') {
                this.actuator.push(ref(value))
            } else {
                this.actuator[i].value = value
            }
        }
    }

    private onGotBinaryMessage(msg: Uint8Array) {
        // Actuator update
        if (msg[0] == 0x01) {
            this.actuatorUpdate(msg[1])
        }
    }

    private wsOnOpen = () => {
        console.log("WS: Open");
    }

    private wsOnClose = () => {
        console.log("WS: Close");
        this.ws.removeEventListener('open', this.wsOnOpen);
        this.ws.removeEventListener('close', this.wsOnClose);
        this.ws.removeEventListener('message', this.wsOnMessage);
        this.ws = null

        setTimeout(() => {
            this.connect()
        }, 10000)
    }

    private wsOnMessage = (event: MessageEvent<string | Blob>) => {

        console.log(event.data)
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
