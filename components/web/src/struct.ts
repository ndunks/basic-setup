/**
 * Struct decode/encode
 */
export default class Struct {
    private bytes: Uint8Array
    private pos: number

    static decode(bytes: Uint8Array) {
        const s = new Struct()
        s.bytes = bytes
        s.pos = 0
        return s
    }

    static encode(capacity: number) {
        const s = new Struct()
        s.bytes = new Uint8Array(capacity)
        s.pos = 0
        return s
    }

    readBool() {
        return this.bytes[this.pos++] === 1
    }

    /** One byte enum */
    readEnum<T>(e: T): T[keyof T] {
        const value = this.bytes[this.pos++];

        if (e[value] !== undefined)
            return value as T[keyof T]
        console.warn(`Unknown enum value ${value}`)
    }

    readUInt32() {
        let num = 0;
        num |= this.bytes[this.pos++] >>> 24 & 0xff
        num |= this.bytes[this.pos++] >>> 16 & 0xff
        num |= this.bytes[this.pos++] >>> 8 & 0xff
        num |= this.bytes[this.pos++] >>> 0 & 0xff
        return num
    }

    readBytes(len: number) {
        const part = this.bytes.slice(this.pos, this.pos + len)
        this.pos += len
        return [...part]
    }

    /** 6 bytes mac address */
    readMac() {
        return this.readBytes(6).map(v => v.toString(16).padStart(2, '0')).join(':')
    }

    /** 4 bytes ipv4 */
    readIpV4() {
        return this.readBytes(4).map((v) => v.toString()).join('.')
    }

    /** Read null-terminated string */
    readString(len: number) {
        let part = this.bytes.slice(this.pos, this.pos + len)
        this.pos += len
        // Check for first null-terminated string
        let endStr = part.indexOf(0)
        if (endStr > -1) {
            part = part.slice(0, endStr)
        }
        // simple byte to string
        return Array.from(part).map(v => String.fromCharCode(v)).join('')
    }

}