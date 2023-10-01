import { createApp } from 'vue'
import App from './App.vue'
import "./main.css"
import 'vuetify/styles'
import { createVuetify } from 'vuetify'
import { aliases, mdi } from 'vuetify/iconsets/mdi-svg'

const vuetify = createVuetify({
    icons: {
        defaultSet: 'mdi',
        aliases,
        sets: {
            mdi,
        },
    },
    // theme: {
    //     defaultTheme: localStorage.getItem('mode') === 'light' ? 'light' : 'dark'
    // }
})

createApp(App).use(vuetify)
.mount('#app')

