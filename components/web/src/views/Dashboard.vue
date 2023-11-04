<script setup lang="ts">
import { mdiLightbulb, mdiLightbulbOn } from '@mdi/js'
import { SensorType } from "@/types";
import { computed } from 'vue';
const switchEnabled = computed(
    () => api.switch.value.names.map((v, i) => ({ v, i }))
        .filter((x) => api.switch.value.status[x.i])
)
const sensorEnabled = computed(
    () => api.sensor.value.names.map((v, i) => ({ v, i }))
        .filter((x) => api.sensor.value.status[x.i])
)
</script>

<template>
    <div class="w-100">
        <div class="mb-3">Switches</div>
        <v-item-group multiple :model-value="api.switchActives.value" selected-class="bg-success"
            @update:model-value="api.updateActuator">
            <v-row Xjustify="center" Xno-gutters>
                <v-col cols="6" sm="4" lg="3" v-for="({ v, i }) of switchEnabled" :key="i">
                    <v-item v-if="api.switch.value.status[i]" :value="i" v-slot="{ isSelected, selectedClass, toggle }">
                        <v-card @click="toggle" :class="selectedClass" rounded
                            class="d-flex flex-column justify-center align-center" height="100">
                            <div>
                                <v-icon size="50" :icon="isSelected ? mdiLightbulbOn : mdiLightbulb" />
                            </div>
                            <div>
                                {{ v }}
                            </div>
                        </v-card>
                    </v-item>
                </v-col>
            </v-row>
        </v-item-group>
        <div class="my-3">Sensors</div>
        <v-row>
            <v-col cols="12" sm="6" md="4" lg="3" v-for="({ v, i }) of sensorEnabled" :key="i">
                <v-card v-if="api.sensor.value.status[i]" :title="v" density="compact" rounded
                    class="d-flex flex-column justify-center align-center" height="100">
                    <v-card-text class="w-100 text-center">
                        <v-progress-circular size="40" v-if="api.sensor.value.types[i] == SensorType.CIRCLE" color="primary"
                            :model-value="api.sensorValues.value[i]">
                            {{ api.sensorValues.value[i] }}
                        </v-progress-circular>
                        <template v-else>
                            {{ api.sensorValues.value[i] }}
                            <v-progress-linear height="20" color="primary" :model-value="api.sensorValues.value[i]">
                            </v-progress-linear>
                        </template>
                    </v-card-text>
                </v-card>
            </v-col>
        </v-row>
    </div>
</template>
