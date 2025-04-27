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
        // Specify the icon folder to be cached
        iconDirs: [resolve(CWD, 'src/assets/svg')],
        // Specify symbolId format
        symbolId: 'svg-icon-[dir]-[name]',
      }),
      // visualizer({
      //   open: true,
      //   gzipSize: true,
      //   brotliSize: true,
      //   filename: 'dist/stats.html',
      // }),
      // viteCompression({
      //   verbose: true,
      //   disable: false,
      //   threshold: 10240,
      //   algorithm: 'gzip',
      //   ext: '.gz',
      // }),
      ViteImageOptimizer(),
    ],
    build: {
      // assetsInlineLimit: 40960,
      rollupOptions: {
        output: {
          chunkFileNames: `assets/js/[name]-[hash].${timestamp}.js`, // Introduce the name of the filename
          entryFileNames: `assets/js/[name]-[hash].${timestamp}.js`, // The name of the package entry file
          // assetFileNames: '[ext]/[name]-[hash].[ext]', // Resource files like fonts, images, etc.
          // Minimizing Split Packages
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
      outDir:'web',           // wrap
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
          target: env.VITE_APP_API_BASE_URL, // Domain name of the interface
          secure: false, // If it is an https interface, you need to configure this parameter
          changeOrigin: true, // If the interface is cross-domain, this parameter configuration is required
          // rewrite: path => path.replace(/^\/api/, ''),
        },
        '/snap': {
          target: env.VITE_APP_API_BASE_URL, // Domain name of the interface
          secure: false, // If it is an https interface, you need to configure this parameter
          changeOrigin: true, // If the interface is cross-domain, this parameter configuration is required
          // rewrite: path => path.replace(/^\/api/, ''),
        },
      },
    },
  })
}
