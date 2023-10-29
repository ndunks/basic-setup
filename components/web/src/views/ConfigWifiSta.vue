<script setup lang="ts">
import { type WifiConfigSta, WifiAuthMode, type WifiScanResult } from '@/types';
import { encodeWifiConfigSta, parseWifiConfigSta, parseWifiScan } from '@/utils';
import { mdiClose, mdiWifiCog, mdiWifiStrength1, mdiWifiStrength1Lock, mdiWifiStrength2, mdiWifiStrength2Lock, mdiWifiStrength3, mdiWifiStrength3Lock, mdiWifiStrength4, mdiWifiStrength4Lock, mdiWifiStrengthLockOutline, mdiWifiStrengthOutline, mdiWifiSync } from '@mdi/js';
import { onMounted, shallowRef } from 'vue';
import { ref } from 'vue';

const emits = defineEmits<{ (event: 'close') }>()

let error = shallowRef([])
let config = ref<WifiConfigSta | null>(null)

let selectedSsid = ref(null)
const loading = ref(false)
const requirePassword = ref(false)
const scanning = ref(false)
const wifiList = shallowRef([] as (WifiScanResult & { icon: typeof mdiWifiStrength1 })[])
const wifiIconOpen = [
    mdiWifiStrengthOutline,
    mdiWifiStrength1,
    mdiWifiStrength2,
    mdiWifiStrength3,
    mdiWifiStrength4,
]

const wifiIconLocked = [
    mdiWifiStrengthLockOutline,
    mdiWifiStrength1Lock,
    mdiWifiStrength2Lock,
    mdiWifiStrength3Lock,
    mdiWifiStrength4Lock,
]

function scan() {
    scanning.value = true
    api.request(WS_MSG_ID_WIFI_STA_SCAN).then(
        raw => {
            wifiList.value = parseWifiScan(raw).map(
                v => {
                    let icon;
                    let iconSet = (v.authmode == WifiAuthMode.open) ? wifiIconOpen : wifiIconLocked
                    if (v.rssi >= 0)
                        icon = iconSet[0]
                    else if (v.rssi > -60)
                        icon = iconSet[4]
                    else if (v.rssi > -70)
                        icon = iconSet[3]
                    else if (v.rssi > -80)
                        icon = iconSet[2]
                    else
                        icon = iconSet[1]
                    return { ...v, icon }
                }
            )
            console.debug(wifiList)
        }
    ).finally(() => scanning.value = false)
}

function connectTo(item: WifiScanResult) {
    if (item.ssid == config.value.ssid) {
        return alert(`Already connected to ${item.ssid}`)
    }
    const payload: typeof config.value = JSON.parse(JSON.stringify(config.value))
    payload.ssid = item.ssid
    if (item.authmode != WifiAuthMode.open) {
        payload.password = prompt(`Please input password for ${item.ssid}`)

    } else {
        payload.password = ""
    }
    loading.value = true
    error.value = []
    const payloadEncoded = encodeWifiConfigSta(payload)
    api.request(WS_MSG_ID_WIFI_CONFIG_STA, payloadEncoded).then(
        updateConfig
    ).catch(e => error.value = [e.message || e])
        .finally(() => loading.value = false)
}

const updateConfig = (cfgRaw: Uint8Array) => {
    const cfg = parseWifiConfigSta(cfgRaw)
    config.value = cfg
    console.debug(config.value)
    selectedSsid.value = null
}

onMounted(() => api.request(WS_MSG_ID_WIFI_CONFIG_STA).then(
    updateConfig
).catch((err) => {
    alert(err.message || err)
    emits('close')
}))

const rule_required = (v) => {
    return !!v || 'Cannot empty'
}

const rules_ap_password = (v: string) => {
    if (!requirePassword.value) return true
    if (!v) return 'Password required'
    if (v.length < 8) return 'Password min length is 8'
    return true
}

</script>
<template>
    <v-card>
        <v-toolbar density="compact">
            <template v-slot:prepend>
                <v-icon :icon="mdiWifiCog"></v-icon>
            </template>
            <v-toolbar-title>Wifi Configuration</v-toolbar-title>
            <template v-slot:append>
                <v-btn :loading="api.isLoading.value" :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-card-text>


            <v-alert v-if="!config">
                <v-progress-linear indeterminate />
            </v-alert>
            <template v-else>
                <p class="text-info">
                    Connect to exisiting wifi networks.
                </p>
                <v-switch label="Enable Wifi Connect" hide-details v-model="config.isEnabled" inset color="success" />
                <template v-if="config.isEnabled">
                    <pre class="text-mutted pa-1 mb-3 border w-100" cols="3">
Mac: {{ config.mac }}
IP: {{ config.net.ip }}
Netmask: {{ config.net.netmask }}
Gateway: {{ config.net.gw }}
SSID: {{ config.ssid }}
Pass: {{ config.password }}</pre>
                    <div class="text-end">
                        <v-btn :loading="scanning" :prepend-icon="mdiWifiSync" @click="scan" color="info">Scan Wifi</v-btn>
                    </div>
                    <v-list>
                        <v-list-item v-for="item of wifiList" :title="item.ssid"
                            :subtitle="`RSSI: ${item.rssi} Auth: ${WifiAuthMode[item.authmode]}`" :prepend-icon="item.icon"
                            @click="connectTo(item)">
                        </v-list-item>
                    </v-list>
                    <!-- <template v-if="selectedSsid">
                            <v-text-field hide-details v-model="config.ssid" label="SSID" />
                            <v-text-field v-if="requirePassword" hide-details v-model="config.password" label="Password" />
                        </template> -->
                </template>


            </template>
        </v-card-text>
        <v-divider />
        <v-card-actions>
            <v-spacer />
            <v-btn :loading="loading" type="submit" color="success" variant="tonal">
                Save
            </v-btn>
        </v-card-actions>
    </v-card>
</template>