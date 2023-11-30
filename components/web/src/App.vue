<script setup lang="ts">
import { ref } from "vue"
import { mdiCog, mdiLock, mdiExitToApp, mdiDipSwitch, mdiKey, mdiRestart, mdiAccessPoint, mdiWifiSettings, mdiLeak, mdiHomeLightbulb } from '@mdi/js'
import ConfigGeneral from "./views/ConfigGeneral.vue";
import ConfigSwitches from "./views/ConfigSwitches.vue";
import Login from "./views/Login.vue";
import ConfigPassword from "./views/ConfigPassword.vue";
import ConfigWifiSta from "./views/ConfigWifiSta.vue";
import { watch } from "vue";
import ConfigWifiAp from "./views/ConfigWifiAp.vue";
import ConfigSensors from "./views/ConfigSensors.vue";
import Dashboard from "./views/Dashboard.vue";

const settingDialog = ref(null)
const connectDialog = ref(true)
const drawer = ref<boolean | null>(null)

function toggleDrawer() {
  drawer.value = !drawer.value
}

const menus = [
  { text: "Wifi Connection", icon: mdiWifiSettings, view: ConfigWifiSta },
  { text: "SoftAP Broadcast", icon: mdiAccessPoint, view: ConfigWifiAp },
  { text: "Switch Config", icon: mdiDipSwitch, view: ConfigSwitches },
  { text: "Sensor Config", icon: mdiLeak, view: ConfigSensors },
  { text: "Password", icon: mdiKey, view: ConfigPassword },
  { text: "Settings", icon: mdiCog, view: ConfigGeneral },
]
if (!sdkconfig.CONFIG_APP_WITH_SENSOR) {
  menus.splice(menus.findIndex(v => v.view == ConfigSensors), 1)
}
let currentSettingView = null

function clickSettings(i: number) {
  currentSettingView = menus[i].view
  drawer.value = false
  settingDialog.value = true
}

watch(api.isConnected, (isConnected) => {
  connectDialog.value = !isConnected
})

async function clickLogin() {
  // try to login with remembered password
  if (await api.autoLogin()) {
    return
  }
  currentSettingView = Login
  settingDialog.value = true
}

function clickLogout() {
  api.logout()
  drawer.value = false
}

function clickRestart() {
  if (!confirm('Restart device?'))
    return

  api.send(WS_MSG_ID_RESTART)
}

</script>

<template>
  <v-app>
    <v-app-bar density="compact">
      <v-app-bar-nav-icon :icon="mdiHomeLightbulb" @click="toggleDrawer"></v-app-bar-nav-icon>
      <v-app-bar-title v-text="api.hostname.value" />
    </v-app-bar>
    <v-navigation-drawer v-model="drawer">
      <v-list>
        <template v-if="api.isLogin.value">
          <v-list-item v-for="({ text, icon }, i) of menus" :prepend-icon="icon" :title="text"
            @click="clickSettings(i)" />
          <v-divider />
          <v-list-item :prepend-icon="mdiExitToApp" @click="clickLogout" title="Logout" />
          <v-list-item :prepend-icon="mdiRestart" @click="clickRestart" title="Restart" />
        </template>
        <template v-else>
          <v-list-item :prepend-icon="mdiLock" @click="clickLogin" title="Login" />
        </template>
        <!-- <v-list-item :prepend-icon="mdiWeatherNight" @click.stop="toggleTheme"
          :title="`${isDarkMode ? 'Light' : 'Dark'} Mode`" /> -->
      </v-list>
    </v-navigation-drawer>
    <v-main>
      <v-container class="fill-height">
        <Dashboard />
      </v-container>
    </v-main>
    <v-dialog v-model="settingDialog" scrollable max-width="500" :close-on-back="false" :close-on-content-click="false">
      <component @close="settingDialog = false" v-if="currentSettingView" :is="currentSettingView" />
    </v-dialog>
    <v-dialog v-model="connectDialog" max-width="360" :close-on-back="false" :close-on-content-click="false">
      <v-card color="warning">
        <v-card-text>
          <VProgressLinear indeterminate class="mb-3" />
          <p>
            Connecting to device on {{ api.host }}
          </p>
        </v-card-text>
      </v-card>
    </v-dialog>
  </v-app>
</template>
