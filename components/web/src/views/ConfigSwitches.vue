<script setup lang="ts">
import { SwitchType } from '@/types';
import { mdiClose, mdiDipSwitch } from '@mdi/js'
import { unref } from 'vue';
import { ref } from 'vue';
const items = api.switch.value.names.map((name, i) => ref({
    name,
    status: api.switch.value.status[i],
    type: api.switch.value.types[i],
}))
const emits = defineEmits<{ (event: 'close') }>()

let error = ref(null)
const buttonType = Object.values(SwitchType).filter((k) => typeof k == 'number')
    .map((k) => ({ title: SwitchType[k], value: k }))

function submit() {
    error.value = null
    api.updateSwitchConfig(items.map(v => unref(v))).then(
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
            <template v-for="(v, i) of items">
                <v-row no-gutters class="mb-2">
                    <div class="pr-2">
                        <v-switch class="text-center" color="success" v-model="v.value.status" hide-details />
                    </div>
                    <v-col>
                        <v-text-field counter density="comfortable" :rounded="0" autocomplete="off" v-model="v.value.name"
                            :label="`Switch ${i + 1}`">
                        </v-text-field>
                    </v-col>
                    <v-col cols="4">
                        <v-select label="type" v-model="v.value.type" density="comfortable" :rounded="0"
                            :items="buttonType" />
                    </v-col>
                </v-row>
            </template>
            <v-divider />
            <div class="mt-2 d-flex">
                <v-spacer />
                <v-btn :loading="api.isLoading.value" type="submit" color="success">
                    Save
                </v-btn>
            </div>
        </v-form>
    </v-sheet>
</template>
