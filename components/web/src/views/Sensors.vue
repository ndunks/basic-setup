<script setup lang="ts">
import { mdiLightbulb, mdiLightbulbOn } from '@mdi/js'
</script>

<template>
    <v-card class="mx-auto" max-width="300" title="Sensors">
        <v-item-group multiple v-if="api.isConnected">
            <v-item v-for="(v,i) of api.sensor.value.names" :key="i" :value="i">
                <v-list-item>
                    <!-- <template #prepend="{ isActive }">
                        <v-switch color="success" class="me-4" :model-value="isActive" hide-details
                            density="comfortable"></v-switch>
                    </template> -->
                    <v-list-item-title>{{ v }}</v-list-item-title>
                    <template #append="{ isActive }">
                        {{ api.sensorValues.value[i] }}
                    </template>
                </v-list-item>
            </v-item>
        </v-item-group>
    </v-card>
</template>
