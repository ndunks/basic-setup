<script setup lang="ts">
import { SensorType } from '@/types';
import { validateName } from '@/utils';
import { mdiClose, mdiLeak } from '@mdi/js'
import { unref } from 'vue';
import { ref } from 'vue';

const items = api.sensor.value.names.map((name, i) => ref({
    name,
    status: api.sensor.value.status[i],
    type: api.sensor.value.types[i],
}))
const emits = defineEmits<{ (event: 'close') }>()
const formValid = ref<boolean | null>(null)

let error = ref(null)
const buttonType = Object.values(SensorType).filter((k) => typeof k == 'number')
    .map((k) => ({ title: SensorType[k], value: k }))

function submit() {
    error.value = null
    if (!formValid.value) return
    api.updateSwitchSensorConfig(items.map(v => unref(v)), WS_MSG_ID_UPDATE_SENSORS).then(
        () => emits('close')
    ).catch(e => error.value = e.message || e)
}

</script>

<template>
    <v-form @submit.prevent="submit" v-model="formValid">
        <v-card density="compact" title="Sensor Configuration" :prepend-icon="mdiLeak">
            <template v-slot:append>
                <v-btn size="sm" variant="plain" :icon="mdiClose" @click="$emit('close')"></v-btn>
            </template>
            <v-divider />
            <v-card-text>
                <v-alert type="error" v-if="error" v-text="error" />
                <template v-for="(v, i) of items">
                    <v-row no-gutters class="mb-1">
                        <div class="pr-2">
                            <v-checkbox class="text-center" color="success" v-model="v.value.status" hide-details />
                        </div>
                        <v-col>
                            <v-text-field variant="underlined" :rules="[validateName]" counter density="comfortable"
                                :rounded="0" autocomplete="off" v-model="v.value.name" :label="`Sensor ${i + 1}`">
                            </v-text-field>
                        </v-col>
                        <v-col cols="4">
                            <v-select variant="underlined" label="type" v-model="v.value.type" density="comfortable"
                                :rounded="0" :items="buttonType" />
                        </v-col>
                    </v-row>
                </template>
            </v-card-text>
            <v-divider />
            <v-card-actions>
                <v-spacer />
                <v-btn :disabled="!formValid" :loading="api.isLoading.value" type="submit" color="success">
                    Save
                </v-btn>
            </v-card-actions>
        </v-card>
    </v-form>
</template>
