<script setup lang="ts">
import { ref } from "vue"
import { mdiHomeLightbulb, mdiWeatherNight, mdiCog, mdiInfinity, mdiSwitch, mdiLock, mdiExitRun, mdiExitToApp } from '@mdi/js'
import { useApi } from "./api";
import Switches from "./views/Switches.vue";
import ConfigGeneral from "./views/ConfigGeneral.vue";
import ConfigSwitches from "./views/ConfigSwitches.vue";
import { useDisplay, useTheme } from "vuetify";
import Login from "./views/Login.vue";
import { computed } from "vue";
import { nextTick } from "vue";
const api = useApi()
const display = useDisplay()
const loginDialog = ref(null)
const settingDialog = ref(null)
const drawer = ref<boolean | null>(null)


function toggleDrawer() {
  drawer.value = !drawer.value
}

const menus = [
  { text: "Settings", icon: mdiCog, view: ConfigGeneral },
  { text: "Switches", icon: mdiSwitch, view: ConfigSwitches }
]

let currentSettingView = null
const drawerLocation = computed(() => (display.mobile.value ? 'top' : 'right'))

function clickSettings(i: number) {
  currentSettingView = menus[i].view
  drawer.value = false
  settingDialog.value = true
  // if (!api.isLogin.value) {
  //   // loginDialog.value = true
  // } else {
  //   // loginDialog.value = false
  //   drawer.value = true
  // }
}

async function clickLogin() {
  const remembered = localStorage.getItem("remember")
  if (remembered) {
    // try to login directly
    const loginSuccess = await api.login(remembered).catch(
      e => {
        // Login with remembered password failed, clear it
        localStorage.removeItem("remember")
        return false
      }
    )
    if (loginSuccess) return
  }

  currentSettingView = Login
  settingDialog.value = true
}

function clickLogout() {
  api.logout()
  drawer.value = false
}

</script>

<template>
  <v-app>
    <v-app-bar density="compact">
      <!-- <template v-slot:prepend>
        <v-icon :icon="mdiHomeLightbulb" />
      </template>-->
      <v-app-bar-nav-icon @click="toggleDrawer"></v-app-bar-nav-icon>
      <v-app-bar-title v-text="api.hostname.value" />

      <!-- <template v-slot:append>
        <v-btn @click="toggleTheme" :prepend-icon="mdiWeatherNight" :active="theme.global.name.value == 'dark'"
          selected-class="bg-success">
          {{ theme.global.name.value }}
        </v-btn>
        <v-btn :icon="mdiCog" @click="drawer = !drawer" />
      </template> -->
    </v-app-bar>
    <v-navigation-drawer v-model="drawer" :location="drawerLocation" temporary>
      <v-list>
        <template v-if="api.isLogin.value">
          <v-list-item v-for="({ text, icon }, i) of menus" :prepend-icon="icon" :title="text"
            @click="clickSettings(i)" />
          <v-divider />
          <v-list-item :prepend-icon="mdiExitToApp" @click.stop="clickLogout" title="Logout" />
        </template>
        <template v-else>
          <v-divider />
          <v-list-item :prepend-icon="mdiLock" @click.stop="clickLogin" title="Login" />
        </template>
        <!-- <v-list-item :prepend-icon="mdiWeatherNight" @click.stop="toggleTheme"
          :title="`${isDarkMode ? 'Light' : 'Dark'} Mode`" /> -->
      </v-list>
    </v-navigation-drawer>
    <v-main>
      <v-container fluid class="fill-height">
        <v-row justify="center" no-gutters>
          <v-col xl="4" lg="5" md="6" cols="12">
            <Switches />
          </v-col>
        </v-row>
      </v-container>
    </v-main>
    <v-dialog v-model="settingDialog" scrollable max-width="500" :close-on-back="false" :close-on-content-click="false">
      <component @close="settingDialog = false" v-if="currentSettingView" :is="currentSettingView" />
    </v-dialog>
  </v-app>
</template>
