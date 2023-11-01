<script setup lang="ts">
import { mdiLightbulb, mdiLightbulbOn } from '@mdi/js'
</script>

<template>
    <v-card class="mx-auto" max-width="300" title="Sensors">
        <v-list lines="two">


            <v-list-item v-for="(v, i) of api.sensor.value.names" :key="i">
                <!-- <template #prepend="{ isActive }">
                        <v-switch color="success" class="me-4" :model-value="isActive" hide-details
                            density="comfortable"></v-switch>
                    </template> -->
                <v-list-item-title>
                    {{ v }}
                </v-list-item-title>
                <v-list-item-subtitle>
                    <v-progress-linear color="primary" :max="1023" :model-value="api.sensorValues.value[i]" />
                </v-list-item-subtitle>
                <template #append="{ isActive }">
                    <v-chip>
                        {{ api.sensorValues.value[i] }}
                    </v-chip>
                </template>
            </v-list-item>
        </v-list>
    </v-card>
</template>
