<script setup lang="ts">
import { useApi } from '@/api';
import { WS_MSG_ID_UPDATE_HOSTNAME } from '@/types';
import { mdiClose, mdiCog, mdiLock, mdiPencil } from '@mdi/js';
import { onMounted, shallowRef } from 'vue';
import { ref } from 'vue';

const emits = defineEmits<{ (event: 'close') }>()

const api = useApi()

let error = shallowRef([])
let name = ref(api.hostname.value)
const loading = ref(false)

function submit() {
    loading.value = true
    error.value = []
    api.requestTruthy(WS_MSG_ID_UPDATE_HOSTNAME, name.value).then(
        () => emits('close')
    ).catch(e => error.value = [e.message || e])
        .finally(() => loading.value = false)
}

</script>
<template>
    <v-form @submit.prevent="submit">
        <v-card>
            <v-toolbar density="compact">
                <template v-slot:prepend>
                    <v-btn :icon="mdiCog"></v-btn>
                </template>

                <v-toolbar-title>General Configuration</v-toolbar-title>

                <template v-slot:append>
                    <v-btn :icon="mdiClose" @click="$emit('close')"></v-btn>
                </template>
            </v-toolbar>
            <v-card-item>
                <v-text-field 
                :error-messages="error"
                density="compact" v-model="name" :prepend-icon="mdiPencil" label="Device Name" />
            </v-card-item>
            <v-card-actions>
                <v-spacer />
                <v-btn :loading="loading" type="submit" color="success">
                    Save
                </v-btn>
            </v-card-actions>
        </v-card>
    </v-form>
</template>