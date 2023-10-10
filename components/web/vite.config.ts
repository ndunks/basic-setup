import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import viteCompression from 'vite-plugin-compression';
import vuetify from 'vite-plugin-vuetify'

// https://vitejs.dev/config/
export default defineConfig({
  build: {
    assetsDir: ""
  },
  plugins: [
    vue(),
    vuetify({ autoImport: true }),
    viteCompression({
      filter: /\.(js|mjs|json|css|html)$/i,
      deleteOriginFile: true,
      threshold: 32,
      compressionOptions: {
        level: 9,
      }
    })
  ],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    }
  }
})
