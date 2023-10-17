<script setup lang="ts">
import { mdiClose, mdiCog, mdiKey, mdiLock, mdiPencil } from '@mdi/js';
import { watch } from 'vue';
import { onMounted, shallowRef } from 'vue';
import { computed } from 'vue';
import { ref } from 'vue';

const emits = defineEmits<{ (event: 'close') }>()

let error = shallowRef(null)
const pass1 = ref('')
const pass2 = ref('')
const ruleRequired = (v) => !!v || 'Required'
const ruleMatch = (v) => v == pass1.value || 'Not match'

function submit() {

    error.value = null
    api.requestTruthy(WS_MSG_ID_UPDATE_PASSWORD, pass1.value).then(
        () => {
            alert('Password changed.')
            emits('close')
        }
    ).catch(e => error.value = e.message || e)
}

</script>
<template>
    <v-sheet>
        <v-toolbar>
            <template v-slot:prepend>
                <v-icon :icon="mdiKey"></v-icon>
            </template>

            <v-toolbar-title>Change Password</v-toolbar-title>

            <template v-slot:append>
                <v-btn :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-form @submit.prevent="submit" class="pa-3">
            <v-text-field counter :rules="[ruleRequired]" clearable v-model="pass1" label="New Password" />
            <v-text-field counter :rules="[ruleRequired, ruleMatch]" clearable v-model="pass2"
                label="Repeat New Password" />
            <v-alert type="error" v-if="error" v-text="error" />
            <div class="mt-2 d-flex">
                <v-spacer />
                <v-btn :loading="api.isLoading.value" type="submit" color="success">
                    Save
                </v-btn>
            </div>
        </v-form>
    </v-sheet>
</template>