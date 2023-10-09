<script setup lang="ts">
import { useApi } from '@/api';
import { mdiLock } from '@mdi/js';
import { onMounted } from 'vue';
import { ref } from 'vue';

const api = useApi()

const emits = defineEmits<{ (event: 'loginSuccess') }>()

let password = ref('')
const loading = ref(false)
function submit() {

}

onMounted(() => {
    if (api.isLogin) {
        return emits('loginSuccess')
    }
})

</script>

<template>
    <v-card title="Login">
        <v-card-text class="mt-3">
            <v-text-field density="compact" :rounded="0" autocomplete="off" v-model="password" :prepend-icon="mdiLock"
                label="Password" type="password" @keydown.enter="submit"></v-text-field>
        </v-card-text>
        <v-divider></v-divider>
        <v-card-actions class="text-center">
            <v-spacer />
            <v-btn :loading="loading" @click="submit" color="success">
                Login
            </v-btn>
        </v-card-actions>
    </v-card>
</template>

