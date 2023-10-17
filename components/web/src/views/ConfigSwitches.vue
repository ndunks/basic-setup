<script setup lang="ts">
import { mdiClose, mdiDipSwitch } from '@mdi/js'
import { ref } from 'vue';
const models = api.switchNames.value.map((v) => ref(v))
const emits = defineEmits<{ (event: 'close') }>()

let error = ref(null)

function submit() {
    error.value = null
    api.updateSwitchNames(models.map(v => v.value)).then(
        () => emits('close')
    ).catch(e => error.value = e.message || e)
}

</script>

<template>
    <v-sheet>
        <v-toolbar>
            <template v-slot:prepend>
                <v-icon :icon="mdiDipSwitch"></v-icon>
            </template>

            <v-toolbar-title>Switch Configuration</v-toolbar-title>

            <template v-slot:append>
                <v-btn :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
        </v-toolbar>
        <v-form @submit.prevent="submit" class="pa-3">
            <v-alert type="error" v-if="error" v-text="error" />

            <v-text-field density="compact" v-for="(v, i) of models" :rounded="0" autocomplete="off" v-model="v.value"
                :label="`Switch ${i + 1}`"></v-text-field>
            <div class="mt-2 d-flex">
                <v-spacer />
                <v-btn :loading="api.isLoading.value" type="submit" color="success">
                    Save
                </v-btn>
            </div>
        </v-form>
    </v-sheet>
</template>
