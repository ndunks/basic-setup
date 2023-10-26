<script setup lang="ts">
import { ref } from "vue"
import { mdiCog, mdiLock, mdiExitToApp, mdiDipSwitch, mdiKeg, mdiKey, mdiRestart, mdiWifiCog, mdiAccessPoint, mdiWifiSettings } from '@mdi/js'
import Switches from "./views/Switches.vue";
import ConfigGeneral from "./views/ConfigGeneral.vue";
import ConfigSwitches from "./views/ConfigSwitches.vue";
import { useDisplay } from "vuetify";
import Login from "./views/Login.vue";
import { computed } from "vue";
import ConfigPassword from "./views/ConfigPassword.vue";
import ConfigWifiSta from "./views/ConfigWifiSta.vue";
import { onMounted } from "vue";
import { watch } from "vue";
import ConfigWifiAp from "./views/ConfigWifiAp.vue";
const display = useDisplay()
const settingDialog = ref(null)
const drawer = ref<boolean | null>(null)

function toggleDrawer() {
  drawer.value = !drawer.value
}

const menus = [
  { text: "Wifi Connection", icon: mdiWifiSettings, view: ConfigWifiSta },
  { text: "SoftAP Broadcast", icon: mdiAccessPoint, view: ConfigWifiAp },
  { text: "Switches", icon: mdiDipSwitch, view: ConfigSwitches },
  { text: "Password", icon: mdiKey, view: ConfigPassword },
  { text: "Settings", icon: mdiCog, view: ConfigGeneral },
]

let currentSettingView = null
const drawerLocation = computed(() => (display.mobile.value ? 'top' : 'left'))

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

const w = watch(api.isConnected, () => {
  clickSettings(1);
  w();
})
// onMounted( () => {
// })

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
      <v-app-bar-nav-icon @click="toggleDrawer"></v-app-bar-nav-icon>
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
