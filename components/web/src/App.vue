<script setup lang="ts">
import { ref } from "vue"
import { mdiHomeLightbulb, mdiWeatherNight, mdiCog } from '@mdi/js'
import { useApi } from "./api";
import Actuator from "./views/Actuator.vue";
import { useTheme } from "vuetify";
import Login from "./views/Login.vue";
const api = useApi()

const loginDialog = ref(null)
const settingDialog = ref(null)
const theme = useTheme()

function toggleTheme() {
  const v = theme.global.name.value == 'dark' ? 'light' : 'dark'
  theme.global.name.value = v
  localStorage.setItem('mode', v)
}

function clickSettings() {
  if (!api.isLogin.value) {
    loginDialog.value = true
    return
  } else {
    loginDialog.value = false
    settingDialog.value = true
  }
}

function checkLogin() {

}

</script>

<template>
  <v-app>
    <v-app-bar>
      <template v-slot:prepend>
        <v-icon :icon="mdiHomeLightbulb" />
      </template>
      <!-- <v-app-bar-nav-icon @click="drawer = !drawer"></v-app-bar-nav-icon> -->

      <v-app-bar-title v-text="api.hostname.value"/>
      <template v-slot:append>
        <v-btn @click="toggleTheme" :prepend-icon="mdiWeatherNight" :active="theme.global.name.value == 'dark'"
          selected-class="bg-success">
          {{ theme.global.name.value }}
        </v-btn>
        <v-btn :icon="mdiCog" @click="clickSettings" />
      </template>

    </v-app-bar>
    <v-main>
      <v-container fluid class="fill-height">
        <v-row justify="center" no-gutters>
          <v-col xl="4" lg="5" md="6" cols="12">
            <Actuator />
          </v-col>
        </v-row>
      </v-container>
    </v-main>

    <v-dialog v-model="loginDialog" max-width="400">
      <Login @login-success="clickSettings" />
    </v-dialog>
  </v-app>
</template>
file:///home/rifin/works/maxsol/maxblast-dash/src/views/Dash.vue
