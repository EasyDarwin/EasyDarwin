import { fileURLToPath, URL } from 'node:url'
import { resolve } from 'node:path'
import * as process from 'node:process'
import { defineConfig, loadEnv } from 'vite'
import vue from '@vitejs/plugin-vue'
// import VueDevTools from 'vite-plugin-vue-devtools'
import Components from 'unplugin-vue-components/vite'
import { AntDesignVueResolver } from 'unplugin-vue-components/resolvers'
import UnoCSS from 'unocss/vite'
import { createSvgIconsPlugin } from 'vite-plugin-svg-icons'
import { visualizer } from 'rollup-plugin-visualizer'
import viteCompression from 'vite-plugin-compression'
import { ViteImageOptimizer } from 'vite-plugin-image-optimizer'
const CWD = process.cwd()
const timestamp = Date.parse(new Date())
// https://vitejs.dev/config/
export default ({ mode }) => {
  const env = loadEnv(mode, CWD)
  // console.log(mode, env, CWD)
  return defineConfig({
    plugins: [
      vue(),
      // VueDevTools(),
      Components({
        resolvers: [
          AntDesignVueResolver({
            importStyle: false,
          }),
        ],
      }),
      UnoCSS(),
      createSvgIconsPlugin({
        iconDirs: [resolve(CWD, 'src/assets/svg')],
        symbolId: 'svg-icon-[dir]-[name]',
      }),
      ViteImageOptimizer(),
    ],
    build: {
      rollupOptions: {
        output: {
          chunkFileNames: `assets/js/[name]-[hash].${timestamp}.js`, 
          entryFileNames: `assets/js/[name]-[hash].${timestamp}.js`, 
          manualChunks(id) {
            if (id.includes('node_modules')) {
              return id
                .toString()
                .split('node_modules/')[1]
                .split('/')[0]
                .toString()
            }
          },
        },
      },
      outDir:'./Win/web',           // 打包到win 下
      // outDir:'./Linux/web',      // 打包到Linux 下
    },
    css: {
      preprocessorOptions: {
        less: {
          javascriptEnabled: true,
        },
      },
    },
    resolve: {
      alias: {
        '@': fileURLToPath(new URL('./src', import.meta.url)),
      },
    },
    server: {
      open: true,
      port: 3001,
      proxy: {
        '/api/v1': {
          target: env.VITE_APP_API_BASE_URL, 
          secure: false, 
          changeOrigin: true,
        },
        '/snap': {
          target: env.VITE_APP_API_BASE_URL,
          secure: false, 
          changeOrigin: true,
        },
      },
    },
  })
}
