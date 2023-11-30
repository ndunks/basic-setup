#!/bin/env node

const fs = require("node:fs")

const sdkconfigList = fs.readFileSync(`${__dirname}/../../sdkconfig`, 'utf8')
    .split('\n')
    .map(v => v.trim())
    .filter(v => v.length && v[0] != '#')
    .filter(v => v.startsWith('CONFIG_APP'))
    .map(v => v.split('='))
    .map(([key, value]) => {
        if (value[0] != '"') {
            if (isNaN(value)) {

                if (value == 'y') {
                    value = 'true'
                } else if (value == 'n') {
                    value = 'false'
                } else {
                    value = JSON.stringify(value)
                }
            }
        }

        return `    ${key}: ${value}`

    })

const sdkconfigStr = `
var sdkconfig: Record<string, any> = globalThis.sdkconfig = {
${sdkconfigList.join(",\n")}
}
`

const configFile = `${__dirname}/src/sdkconfig.ts`
fs.writeFileSync(configFile, sdkconfigStr, 'utf8')