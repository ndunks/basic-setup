<script setup lang="ts">
import { mdiInfinity, mdiLightbulb, mdiLightbulbOn, mdiSwitch } from '@mdi/js'
import { useApi } from "@/api";
import { ref } from 'vue';
const api = useApi()
const models = api.switchNames.value.map((v) => ref(v))

</script>

<template>
    <v-card title="Switch Configuration">
        <v-card-item>
            <v-form>
                <v-text-field density="compact" v-for="(v, i) of models" :rounded="0" autocomplete="off" Xv-model="v"
                    :label="`Switch ${i + 1}`"></v-text-field>
            </v-form>
        </v-card-item>
    </v-card>
</template>
