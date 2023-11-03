import Struct from "./struct";
import { type AppConfig, type WifiConfigAp, type WifiConfigSta, WifiMode, WifiAuthMode, type IpInfo, type WifiScanResult, SensorType, SwitchType } from "./types";

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
    writer.writeBool(b.isEnabled)
    writer.writeMac(b.mac)
    writer.writeIpV4(b.net.ip)
    writer.writeIpV4(b.net.netmask)
    writer.writeIpV4(b.net.gw)
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
    writer.writeIpV4(b.net.ip)
    writer.writeIpV4(b.net.netmask)
    writer.writeIpV4(b.net.gw)

    writer.writeString(b.ssid, 32)
    writer.writeString(b.password, 64)
    writer.writeBool(b.autoConnect)
    return writer.getBytes()
}

export function parseWifiScan(b: Uint8Array) {
    const results: WifiScanResult[] = []
    const reader = Struct.decode(b);
    // sizeof(struct ws_scan_result) = 44

    while (reader.remainingBytes() >= 44) {
        results.push({
            bssid: reader.readMac(),
            ssid: reader.readString(33),
            rssi: reader.readByteSigned(),
            authmode: reader.readEnum(WifiAuthMode)
        })
        // remove padding
        reader.readBytes(3)
    }
    return results;
}

export function parseAppConfigStruct(b: Uint8Array): AppConfig {


    let configVersion: number,
        switchLen: number,
        sensorLen: number
    const reader = Struct.decode(b)

    configVersion = reader.readUInt16()
    switchLen = reader.readByte()
    sensorLen = reader.readByte()
    const cfg: AppConfig = {
        configVersion,
        switchLen,
        sensorLen,
        switchValues: reader.readByte(),
        switchCfg: [...new Array(switchLen)].map((_, i) => {
            return parseSensorSwitchCfg(reader.readByte(), SwitchType)
        }),
        sensorCfg:  [...new Array(sensorLen)].map((_, i) => {
            return parseSensorSwitchCfg(reader.readByte(), SensorType)
        }),
        reserved1: reader.readByte(),
        sensor_delay: reader.readUInt16(),
        reserved2: reader.readUInt16(),
        hostname: reader.readString(TCPIP_HOSTNAME_MAX_SIZE),
        switches: [...new Array(switchLen)].map((_, i) => {
            return reader.readString(APP_NAME_MAX_SIZE)
        }),
        sensors: [...new Array(sensorLen)].map((_, i) => {
            return reader.readString(APP_NAME_MAX_SIZE)
        }),
    }
    console.debug(cfg)
    return cfg
}

export function parseSensorSwitchCfg<T extends typeof SensorType | typeof SwitchType>(
    byte: number, t: T) {
    const status = (byte & 0x80) == 0x80
    let type: T[keyof T] = t[t[0]]
    const enumVal = byte & 0x7f
    const x = SwitchType[0]
    if (enumVal in t)
        type = enumVal as unknown as T[keyof T]
    return {
        status, type
    }
}

// Create array contain index of active bytes
export function bitsFilterOn(byte: number): number[] {
    const actives = []
    for (let i = 0; i < 8; i++) {
        if (byte & 1 << i)
            actives.push(i)
    }
    return actives
}

export function bitsFilterArraytoByte(values: number[]): number {
    return values.reduce((c, v, i) => c | (1 << v) >>> 0, 0)
}
