<script setup lang="ts">
import { useApi } from '@/api';
import { mdiEye, mdiEyeOff, mdiLock } from '@mdi/js';
import { shallowRef } from 'vue';
import { onMounted } from 'vue';
import { ref } from 'vue';

const api = useApi()

const emits = defineEmits<{ (event: 'close') }>()
const remembered = localStorage.getItem("remember")
let password = ref(remembered || '')
let rememberPassword = ref(!!remembered)
let error = shallowRef([])

const loading = ref(false)
const passwordVisible = ref(false)
function submit() {
    loading.value = true
    error.value = []
    api.login(password.value).then(
        () => {
            if (rememberPassword.value)
                localStorage.setItem("remember", password.value)
            emits('close')
        }
    ).catch(e => error.value = [e.message || e])
        .finally(() => loading.value = false)
}

</script>

<template>
    <v-form @submit.prevent="submit">
        <v-card title="Login" :prepend-icon="mdiLock">
            <v-card-text class="mt-3">
                <v-text-field counter :append-icon="passwordVisible ? mdiEye : mdiEyeOff" Xdensity="compact"
                    autocomplete="off" v-model="password" label="Password" :error-messages="error"
                    :type="passwordVisible ? 'text' : 'password'"
                    @click:append="passwordVisible = !passwordVisible"></v-text-field>
                <v-checkbox hide-details v-model="rememberPassword" label="Remember password" />
            </v-card-text>

            <v-divider></v-divider>
            <v-card-actions class="text-center">
                <v-spacer />
                <v-btn :loading="loading" type="submit" color="success">
                    Login
                </v-btn>
            </v-card-actions>
        </v-card>
    </v-form>
</template>

