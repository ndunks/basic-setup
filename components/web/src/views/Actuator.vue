<script setup lang="ts">
import { mdiLightbulb, mdiLightbulbOn } from '@mdi/js'
import { useApi } from "../api";
const api = useApi()

</script>

<template>
    <v-card class="mx-auto" max-width="300" title="Switches">
        <v-item-group :model-value="api.actuator.value" @update:model-value="api.updateActuator" multiple>
            <v-item v-for="v of api.actuatorOptions" :key="v" :value="v" v-slot="{ isSelected, toggle }">
                <v-list-item @click="toggle" :active="isSelected" color="success">
                    <template #prepend="{ isActive }">
                        <v-switch color="success" class="me-4" :model-value="isActive" hide-details
                            density="comfortable"></v-switch>
                    </template>
                    <v-list-item-title>{{ v }}</v-list-item-title>
                    <template #append="{ isActive }">
                        <v-icon :icon="isActive ? mdiLightbulbOn : mdiLightbulb" />
                    </template>
                </v-list-item>
            </v-item>
        </v-item-group>
    </v-card>
</template>
