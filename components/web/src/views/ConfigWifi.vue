<script setup lang="ts">
import { parseWifiConfigInfo } from '@/utils';
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
let name = ref(api.hostname.value)
const loading = ref(false)
const showReset = ref(false)

function submit() {
    loading.value = true
    error.value = []
    api.requestTruthy(WS_MSG_ID_UPDATE_HOSTNAME, name.value).then(
        () => emits('close')
    ).catch(e => error.value = [e.message || e])
        .finally(() => loading.value = false)
}

function onResetConfig() {
    if (!confirm('Are you sure want to reset device configuration?'))
        return
    api.send(WS_MSG_ID_RESET_CONFIG)
    emits('close')
}
onMounted(() => api.request(WS_MSG_ID_WIFI_CONFIG).then(
    cfgRaw => {
        const info = parseWifiConfigInfo(cfgRaw)
        console.debug(info)
    }
).catch((err) => {
    alert(err.message || err)
    emits('close')
}))
</script>
<template>
    <v-sheet>
        <v-toolbar>
            <template v-slot:prepend>
                <v-icon :icon="mdiWifiCog"></v-icon>
            </template>

            <v-toolbar-title>Wifi Configuration</v-toolbar-title>

            <template v-slot:append>
                <v-btn :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-sheet class="ma-3 my-5" border rounded>
            <v-toolbar>
                <v-switch class="d-flex mx-3" inset color="success">
                    <template #label>
                        <v-toolbar-title>Client Mode</v-toolbar-title>
                    </template>
                </v-switch>
            </v-toolbar>
            <v-form @submit.prevent="submit" class="pa-3">
                <p class="my-5 mx-1 text-info">
                    Connect to exisiting wifi networks.
                </p>
                <v-text-field :error-messages="error" clearable v-model="name" label="Device Name" />
                <v-switch v-model="isDarkMode" label="Dark Mode" />
                <div class="mt-2 d-flex">
                    <v-spacer />
                    <v-btn :loading="loading" type="submit" color="success">
                        Save
                    </v-btn>
                </div>
            </v-form>
        </v-sheet>
        <v-sheet class="ma-3 my-5" border rounded>
            <v-toolbar>
                <v-switch class="d-flex mx-3" inset color="success">
                    <template #label>
                        <v-toolbar-title>Access Point Mode</v-toolbar-title>
                    </template>
                </v-switch>


            </v-toolbar>
            <v-form @submit.prevent="submit" class="pa-3">
                <p class="my-5 mx-1 text-info">
                    Broadcast wifi and let others to connect.
                </p>
                <v-text-field :error-messages="error" clearable v-model="name" label="Device Name" />
                <v-switch v-model="isDarkMode" label="Allow No Password" />
                <div class="mt-2 d-flex">
                    <v-spacer />
                    <v-btn :loading="loading" type="submit" color="success">
                        Save
                    </v-btn>
                </div>
            </v-form>
        </v-sheet>
    </v-sheet>
</template>