#!/bin/env node

/**
 * Combine dist file, and creating the structs
 */

const { writeSync, createReadStream, writeFileSync, closeSync, openSync, readFileSync, readdirSync } = require("fs")
const { resolve } = require("path")
const { createInterface } = require("readline");
const { getType } = require("mime");


const targetHeader = resolve(__dirname, "include/web.h");
const sourceDirs = resolve(__dirname, "dist");
const targetFile = openSync(resolve(__dirname, "web.bin"), "w+");

/** @type {Array<{ name: string, offset: number, size: number }>} */
const allStructs = []

let lastOffset = 0
readdirSync(sourceDirs).forEach(name => {
    const buffer = readFileSync(resolve(sourceDirs, name))
    allStructs.push({
        name,
        offset: lastOffset,
        size: buffer.byteLength
    })
    writeSync(targetFile, buffer);
    lastOffset += buffer.byteLength;
})

console.log(allStructs)
const totalSize = allStructs.reduce((c, v) => c + v.size, 0)
console.log('File: %d, Total Size: %d KB', allStructs.length, totalSize / 1024)
closeSync(targetFile)

// Writing header files

/** @type {string[]} */
const headerBuffers = [];

function writeStructs() {

    let str = '', isLast = ''
    /** @type {string[]} */
    const mimes = [];
    /** @type {string[]} */
    const web_mimes = []
    /** @type {string[]} */
    const web_files = []
    let indexOffset = 0

    web_files.push(`\n    static const webfs_t web_files[] = {`)
    allStructs.forEach(({ name, size, offset }, i, a) => {
        isLast = (i + 1) == a.length
        let isGzip = false
        if (name.endsWith('.gz')) {
            isGzip = true
            name = name.slice(0, -3)
        }
        const typeStr = getType(name) || 'application/octet-stream'
        let typeIndex = mimes.indexOf(typeStr)
        if (typeIndex < 0) {
            typeIndex = mimes.push(typeStr) - 1
        }

        if( name == 'index.html' ){
            indexOffset = i
        }
        
        str = `        {.name = ${JSON.stringify(name)}, .type = web_mimes[${typeIndex}],` +
        ` .gzip = ${isGzip ? 'true' : 'false'}, .offset = ${offset}, .size = ${size}}`
        web_files.push(str + (isLast ? '};\n' : ','))
    })
    web_files.push(`#define INDEX_HTML_OFS ${indexOffset}`)

    web_mimes.push(`\n    static const char *const web_mimes[] = {`)
    mimes.forEach((name, i, a) => {
        isLast = (i + 1) == a.length
        str = `        ${JSON.stringify(name)}`
        web_mimes.push(str + (isLast ? '};' : ','))
    })
    headerBuffers.push(...web_mimes)
    headerBuffers.push(...web_files)
}

writeHeader()

async function writeHeader() {
    let isWritten = false;
    let isSkip = false;
    const input = createReadStream(targetHeader, 'ascii')
    const reader = createInterface({ input, crlfDelay: Infinity })
    for await (const line of reader) {
        if (isSkip) {
            if (line.includes('/* GENERATED CODE END')) {
                headerBuffers.push(`    // Total size: ${totalSize} bytes\n    // Generated at ${new Date().toLocaleString()}`)
                headerBuffers.push(line)
                isSkip = false
            }
        } else {
            headerBuffers.push(line)
            if (!isWritten && line.includes('/* GENERATED CODE START')) {
                writeStructs()
                isSkip = true
                isWritten = true
            }
        }
    }
    reader.close()
    input.close()
    writeFileSync(targetHeader, headerBuffers.join("\n"))
    console.log(targetHeader, ' updated');
}
