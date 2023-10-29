<script setup lang="ts">
import { mdiClose, mdiCog } from '@mdi/js';
import { watch } from 'vue';
import { shallowRef } from 'vue';
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
</script>
<template>
    <v-sheet>
        <v-toolbar>
            <template v-slot:prepend>
                <v-icon :icon="mdiCog"></v-icon>
            </template>

            <v-toolbar-title>Settings</v-toolbar-title>

            <template v-slot:append>
                <v-btn :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-form @submit.prevent="submit" class="pa-3">
            <v-text-field :error-messages="error" clearable v-model="name" label="Device Name" />
            <v-switch v-model="isDarkMode" label="Dark Mode" />
            <v-alert v-if="showReset" variant="elevated" type="warning">
                Reset Configuration including password and switches
                <v-divider class="my-3" />
                <v-btn color="danger" @click="onResetConfig" variant="outlined">Reset</v-btn>
            </v-alert>
            <div class="mt-2 d-flex">
                <v-btn v-if="!showReset" variant="text" color="warning" size="small" @click="showReset = true">Reset
                    Configuration</v-btn>
                <v-spacer />
                <v-btn :loading="loading" type="submit" color="success">
                    Save
                </v-btn>
            </div>
        </v-form>
    </v-sheet>
</template>