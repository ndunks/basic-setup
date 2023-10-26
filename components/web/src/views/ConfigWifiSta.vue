<script setup lang="ts">
import { WifiMode, type WifiConfigSta, WifiAuthMode } from '@/types';
import { parseWifiConfigSta } from '@/utils';
import { mdiClose, mdiCog, mdiLock, mdiPencil, mdiWifiCog } from '@mdi/js';
import { watch } from 'vue';
import { onMounted, shallowRef } from 'vue';
import { computed } from 'vue';
import { ref } from 'vue';
import { useTheme } from 'vuetify';

const emits = defineEmits<{ (event: 'close') }>()

let error = shallowRef([])
let config = ref<WifiConfigSta | null>(null)

let name = ref(api.hostname.value)
const loading = ref(false)
const requirePassword = ref(false)

function submit() {
    loading.value = true
    error.value = []
    api.requestTruthy(WS_MSG_ID_UPDATE_HOSTNAME, name.value).then(
        () => emits('close')
    ).catch(e => error.value = [e.message || e])
        .finally(() => loading.value = false)
}

const updateConfig = (cfgRaw: Uint8Array) => {
    const cfg = parseWifiConfigSta(cfgRaw)
    config.value = cfg
    console.debug(config.value)
}

function onResetConfig() {
    if (!confirm('Are you sure want to reset device configuration?'))
        return
    api.send(WS_MSG_ID_RESET_CONFIG)
    emits('close')
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
    <v-sheet>
        <v-toolbar>
            <template v-slot:prepend>
                <v-icon :icon="mdiWifiCog"></v-icon>
            </template>

            <v-toolbar-title>Wifi Configuration</v-toolbar-title>

            <template v-slot:append>
                <v-btn :loading="api.isLoading.value" :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-alert v-if="!config">
            <v-progress-linear indeterminate />
        </v-alert>
        <template v-else>
            <v-form @submit.prevent="submit" class="pa-3">
                <p class="my-5 mx-1 text-info">
                    Connect to exisiting wifi networks.
                </p>
                <div class="d-flex">
                    <v-text-field hide-details v-model="config.ssid" label="SSID" />
                    <v-text-field hide-details v-model="config.password" label="Password" />
                </div>
                <v-switch color="info" v-model="config.autoConnect" label="Auto Connect" />
                <div class="mt-2 d-flex">
                    <v-spacer />
                    <v-btn :loading="loading" type="submit" color="success">
                        Save
                    </v-btn>
                </div>
            </v-form>

        </template>
    </v-sheet>
</template>