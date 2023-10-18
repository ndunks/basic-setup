import { type AppConfig, type WifiConfigInfo, type WifiInfo, WifiMode, WifiAuthMode } from "./types";

export function parseWifiConfigInfo(b: Uint8Array): WifiConfigInfo {

    let mode = WifiMode.Unknown

    let ofs = 0 + 3 // 3 byte padding
    const modeNum = readUint32(b);
    ofs += 4
    if (modeNum in WifiMode)
        mode = WifiMode[modeNum] as unknown as WifiMode
    else
        console.warn(`Unknown wifi mode ${modeNum}`)

    const sta = parseWifiInfo(b.slice(ofs), 'sta')
    ofs += 124 // wifi info len
    const ap = parseWifiInfo(b.slice(ofs), 'ap')
    return { mode, sta, ap }
}

export function parseWifiInfo(b: Uint8Array, type: 'ap' | 'sta'): WifiInfo {
    let ofs = 0
    // mac is 6 byte, 2 byte is padding
    const mac = readMac(b.slice(ofs, ofs + 8))
    ofs += 8
    const ip = readIp(b.slice(ofs, ofs + 4))
    ofs += 4
    const netmask = readIp(b.slice(ofs, ofs + 4))
    ofs += 4
    const gw = readIp(b.slice(ofs, ofs + 4))
    ofs += 4
    const ssid = readStr(b.slice(ofs, ofs + 32))
    ofs += 32
    const password = readStr(b.slice(ofs, ofs + 64))
    ofs += 64
    let auto_connect: boolean,
        ap_started: boolean,
        ap_authmode: WifiAuthMode

    if (type == 'sta') {
        auto_connect = b[ofs] == 1;
        ofs += 4 // bool is 1 byte, 3 byte is padding
    } else {
        ap_started = b[ofs] == 1;
        ofs += 4 // bool is 1 byte, 3 byte is padding
        const modeNum = readUint32(b.slice(ofs, ofs + 4))
        ofs += 4
        if (modeNum in WifiAuthMode)
            ap_authmode = WifiAuthMode[modeNum] as unknown as WifiAuthMode
        else
            console.warn(`Unknown auth mode ${modeNum}`)
    }
    ofs += 1 + 4
    return {
        mac, ssid, password, auto_connect, ap_started, ap_authmode,
        net: { ip, netmask, gw },
    }

}

function readStr(b: Uint8Array) {
    // Check null terminated string
    let endStr = b.indexOf(0)
    if (endStr > -1) {
        b = b.slice(0, endStr)
    }
    return Array.from(b).map(v => String.fromCharCode(v)).join('')
}

function readMac(b: Uint8Array): string {
    return [...b.slice(0, 6)].map(v => v.toString(16).padStart(2, '0')).join(':')
}

function readIp(b: Uint8Array): string {
    return [...b.slice(0, 4)].map((v) => v.toString()).join('.')
}

function readUint32(b: Uint8Array): number {
    return b[0] >>> 24 & 0xff
        | b[1] >>> 16 & 0xff
        | b[2] >>> 8 & 0xff
        | b[3] >>> 0 & 0xff
}

export function parseAppConfigStruct(b: Uint8Array): AppConfig {


    let configVersion: number,
        switchLen: number,
        sensorLen: number,
        switchValues: number,
        hostname: string,
        switches: string[],
        sensors: string[];
    let ofs = 0;

    function readStr(maxLen: number) {
        const str = [...b.slice(ofs, ofs + maxLen)]
        ofs += maxLen
        // Check null terminated string
        let endStr = str.indexOf(0)
        if (endStr > -1) {
            str.splice(endStr)
        }
        return str.map(v => String.fromCharCode(v)).join('')
    }


    // From Bigendian, readUint16
    configVersion = (((b[ofs]) & 0xff) >>> 0) | ((b[ofs + 1] << 8) & 0xff) >>> 0;
    ofs += 2
    switchLen = b[ofs++];
    sensorLen = b[ofs++];
    switchValues = b[ofs++];
    // if( configVersion != 1 ){
    //     console.warn('Unsupported config version', configVersion)
    //     return null;
    // }
    hostname = readStr(TCPIP_HOSTNAME_MAX_SIZE)
    switches = [...new Array(switchLen)].map((_, i) => {
        return readStr(APP_NAME_MAX_SIZE)
    })
    sensors = [...new Array(sensorLen)].map((_, i) => {
        return readStr(APP_NAME_MAX_SIZE)
    })

    const cfg: AppConfig = {
        configVersion,
        switchLen,
        sensorLen,
        switchValues,
        hostname,
        switches,
        sensors,
    }
    console.debug(cfg)
    return cfg
}


export function bitsOnToArray(byte: number): number[] {
    const actives = []
    for (let i = 0; i < 8; i++) {
        if (byte & 1 << i)
            actives.push(i)
    }
    return actives
}

export function bitsArraytoByte(values: number[]): number {
    return values.reduce((c, v, i) => c | (1 << v) >>> 0, 0)
}
