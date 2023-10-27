<script setup lang="ts">
import { WifiMode, type WifiConfigAp, WifiAuthMode } from '@/types';
import { encodeWifiConfigAp, parseWifiConfigAp } from '@/utils';
import { mdiClose, mdiCog, mdiLock, mdiPencil, mdiWifiCog } from '@mdi/js';
import { watch } from 'vue';
import { onMounted, shallowRef } from 'vue';
import { computed } from 'vue';
import { ref } from 'vue';
import { useTheme } from 'vuetify';
const theme = useTheme()
const isDarkMode = ref(theme.global.name.value == 'dark')
watch(isDarkMode, () => {
    const v = isDarkMode.value ? 'dark' : 'light'
    theme.global.name.value = v
    localStorage.setItem('mode', v)
})
const emits = defineEmits<{ (event: 'close') }>()

let error = shallowRef([])
let config = ref<WifiConfigAp | null>(null)

let name = ref(api.hostname.value)
let formValue = ref<boolean | null>(null)

const loading = ref(false)
const apAllowNoPassword = ref(false)

function submit() {
    if (!formValue.value) {
        return
    }
    const payload = encodeWifiConfigAp(config.value)
    error.value = []
    loading.value = true
    //new Promise(r => setTimeout(r, 1000))

    api.request(WS_MSG_ID_WIFI_CONFIG_AP, payload).then(
        () => emits('close')
    ).catch(e => error.value = [e.message || e])
        .finally(() => loading.value = false)
}

const updateConfig = (cfgRaw: Uint8Array) => {
    const cfg = parseWifiConfigAp(cfgRaw)
    config.value = cfg
    console.debug(config.value)
    apAllowNoPassword.value = cfg.authmode == WifiAuthMode.open
}


onMounted(() => api.request(WS_MSG_ID_WIFI_CONFIG_AP).then(
    updateConfig
).catch((err) => {
    alert(err.message || err)
    emits('close')
}))

const rule_required = (v) => {
    if (!config.value.isEnabled) return true
    return !!v || 'Cannot empty'
}

const rules_ap_password = (v: string) => {
    if (apAllowNoPassword.value) return true
    if (!v) return 'Password required'
    if (v.length < 8) return 'Password min length is 8'
    if (v.length > 64) return 'Password max length is 64'
    return true
}

</script>
<template>
    <v-sheet>
        <v-toolbar>
            <template v-slot:prepend>
                <v-icon :icon="mdiWifiCog"></v-icon>
            </template>

            <v-toolbar-title>SoftAP Configuration</v-toolbar-title>

            <template v-slot:append>
                <v-btn :loading="api.isLoading.value" :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-alert v-if="!config">
            <v-progress-linear indeterminate />
        </v-alert>
        <template v-else>
            <v-form @submit.prevent="submit" class="pa-3" v-model="formValue">
                <p class="my-5 mx-1 text-info">
                    Broadcast wifi and let others to connect.
                </p>

                <v-switch label="Enable Broadcast SoftAP" v-model="config.isEnabled" inset color="success" />
                <template v-if="config.isEnabled">
                    <v-text-field :rules="[rule_required]" v-model="config.ssid" label="SSID" />
                    <v-switch color="error" v-model="apAllowNoPassword" label="No Password" />
                    <v-text-field :rules="[rules_ap_password]" v-if="!apAllowNoPassword" v-model="config.password"
                        label="Password" />
                </template>
                <div class="mt-2 d-flex">
                    <v-spacer />
                    <v-btn :disabled="!formValue" :loading="loading" type="submit" color="success">
                        Save
                    </v-btn>
                </div>
            </v-form>
        </template>
    </v-sheet>
</template>