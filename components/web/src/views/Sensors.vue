<script setup lang="ts">
import { mdiPencil } from '@mdi/js'
defineEmits<{ (event: 'edit') }>()
</script>

<template>
    <v-card class="mx-auto" max-width="300" title="Sensors">
        <template #append v-if="api.isLogin.value">
            <v-btn variant="plain" @click="$emit('edit')" class="text-warning" title="Edit" size="sm" :icon="mdiPencil" />
        </template>
        <v-list lines="two">
            <v-list-item v-for="(v, i) of api.sensor.value.names" :key="i">
                <v-list-item-title>
                    {{ v }}
                </v-list-item-title>
                <v-list-item-subtitle>
                    <v-progress-linear color="primary" :model-value="api.sensorValues.value[i]" />
                </v-list-item-subtitle>
                <template #append>
                    <v-chip>
                        {{ api.sensorValues.value[i] }}
                    </v-chip>
                </template>
            </v-list-item>
        </v-list>
    </v-card>
</template>
