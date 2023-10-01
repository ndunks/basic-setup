<script setup lang="ts">
import { onMounted } from "vue";
import { ref } from "vue"
import { mdiHome, mdiBolt } from '@mdi/js'
const counter = ref(null)
onMounted(() => {
  setInterval(() => {
    counter.value.innerText = Date.now();
  }, 1000)
})

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
            <v-checkbox>TEST</v-checkbox>
            <v-btn color="success" :icon="mdiHome" />
          </v-form>
        </v-col>
      </v-row>
    </v-container>
  </v-app>
</template>
