import Struct from "./struct";
import { type AppConfig, type WifiConfigAp, type WifiConfigSta, WifiMode, WifiAuthMode, type IpInfo } from "./types";

export function parseWifiConfigAp(b: Uint8Array): WifiConfigAp {
    const reader = Struct.decode(b)
    const cfg: WifiConfigAp = {
        isEnabled: reader.readBool(),
        mac: reader.readMac(),
        net: {
            ip: reader.readIpV4(),
            netmask: reader.readIpV4(),
            gw: reader.readIpV4(),
        },
        ssid: reader.readString(32),
        password: reader.readString(64),
        isStarted: reader.readBool(),
        authmode: reader.readEnum(WifiAuthMode)

    }
    return cfg
}

export function encodeWifiConfigAp(b: WifiConfigAp): Uint8Array {
    // sizeof(struct wifi_config_ap) = 120
    const writer = Struct.encode(120 - 1) // exclude msgId
    //writer.writeByte(WS_MSG_ID_WIFI_CONFIG_AP)


    writer.writeBool(b.isEnabled)
    writer.writeMac(b.mac)
    //net: {
    writer.writeIpV4(b.net.ip)
    writer.writeIpV4(b.net.netmask)
    writer.writeIpV4(b.net.gw)
    //},
    writer.writeString(b.ssid, 32)
    writer.writeString(b.password, 64)
    writer.writeBool(b.isStarted)
    writer.writeByte(b.authmode)

    return writer.getBytes()
}

export function parseWifiConfigSta(b: Uint8Array): WifiConfigSta {
    const reader = Struct.decode(b)
    const cfg: WifiConfigSta = {
        isEnabled: reader.readBool(),
        mac: reader.readMac(),
        net: {
            ip: reader.readIpV4(),
            netmask: reader.readIpV4(),
            gw: reader.readIpV4(),
        },
        ssid: reader.readString(32),
        password: reader.readString(64),
        autoConnect: reader.readBool(),

    }
    return cfg
}

export function encodeWifiConfigSta(b: WifiConfigSta): Uint8Array {
    // sizeof(struct wifi_config_sta) = 120
    const writer = Struct.encode(120 - 1) // exclude msgId

    writer.writeBool(b.isEnabled)
    writer.writeMac(b.mac)
    //net: {
    writer.writeIpV4(b.net.ip)
    writer.writeIpV4(b.net.netmask)
    writer.writeIpV4(b.net.gw)
    //},
    writer.writeString(b.ssid, 32)
    writer.writeString(b.password, 64)
    writer.writeBool(b.autoConnect)
    return writer.getBytes()
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
