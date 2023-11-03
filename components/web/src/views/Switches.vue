<script setup lang="ts">
import { mdiLightbulb, mdiLightbulbOn, mdiPencil } from '@mdi/js'
defineEmits<{(event: 'edit')}>()
</script>

<template>
    <v-card class="mx-auto" max-width="300" title="Switches">
        <template #append v-if="api.isLogin.value">
            <v-btn @click="$emit('edit')" class="text-warning" title="Edit" size="sm" :icon="mdiPencil"/>
        </template>
        <v-item-group multiple v-if="api.isConnected" :model-value="api.switchActives.value" @update:model-value="api.updateActuator">
            <v-item v-for="(v,i) of api.switch.value.names" :key="i" :value="i" v-slot="{ isSelected, toggle }">
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
        <v-alert v-else type="info">
            Not Connected
        </v-alert>
    </v-card>
</template>
