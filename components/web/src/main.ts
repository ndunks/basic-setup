import './sdkconfig'
import './global'
import { Api } from "./api"

import { createApp } from 'vue'
import App from './App.vue'
import "./main.css"
import 'vuetify/styles'
import { createVuetify } from 'vuetify'
import { aliases, mdi } from 'vuetify/iconsets/mdi-svg'

declare global {
    var api: Api
}
declare module 'vue' {
    interface ComponentCustomProperties {
        api: Api
    }
}

const vuetify = createVuetify({
    icons: {
        defaultSet: 'mdi',
        aliases,
        sets: {
            mdi,
        },
    },
    theme: {
        defaultTheme: localStorage.getItem('mode') === 'light' ? 'light' : 'dark'
    }
})

createApp(App).use({
    install(app) {
        app.config.globalProperties.api = globalThis.api
    }
}).use(vuetify).mount('#app')

