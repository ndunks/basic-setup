import { TCPIP_HOSTNAME_MAX_SIZE, type AppConfig, APP_NAME_MAX_SIZE } from "./types";

export function parseAppConfigStruct(b: Uint8Array): AppConfig {
   

    let configVersion: number,
        switchLen: number,
        sensorLen: number,
        switchValues: number,
        hostname: string,
        password: string,
        switches: string[],
        sensors: string[];
    let ofs = 0;

    function readStr(maxLen: number) {
        const str = [...b.slice(ofs, ofs + maxLen)]
            .filter(v => !!v).map(v => String.fromCharCode(v)).join('')
        ofs += maxLen
        return str
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
    password = readStr(APP_NAME_MAX_SIZE)
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
        password,
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
