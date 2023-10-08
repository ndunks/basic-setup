<script setup lang="ts">
import { onMounted } from "vue";
import { ref } from "vue"
import { mdiHome, mdiBolt } from '@mdi/js'
import { Api } from "./api";
import { reactive } from "vue";
const counter = ref(null)


const api = new Api('ws://' + (import.meta.env.VITE_API || location.host) + '/ws')
api.connect()
const actuator = api.actuator

// onMounted(() => {
//   setInterval(() => {
//     counter.value.innerText = Date.now();
//   }, 1000)
// })

function onCheck() {
  fetch(import.meta.env.VITE_API).then(
    (r) => console.log(r.headers)
  )
}



</script>

<template>
  <v-app>
    <v-container fluid class="fill-height">
      <v-row justify="center" no-gutters>
        <v-col xl="3" lg="4" md="5" sm="6" xs="10">
          <v-form class="text-center">
            <v-avatar class="mb-4" color="grey-darken-1" size="64"><v-icon :icon="mdiBolt"></v-icon></v-avatar>
            <h3 class="mb-3 text-grey-darken-1">MaxBlast</h3>
            <div ref="counter">...</div>
            <v-btn @click="onCheck" color="success" :icon="mdiHome" />
            <v-checkbox v-for="(v, i) of actuator" :key="i" v-model="v.value">{{ i + 1 }}</v-checkbox>
          </v-form>
        </v-col>
      </v-row>
    </v-container>
  </v-app>
</template>
