import { defineConfig } from 'vite';
import solidPlugin from 'vite-plugin-solid';
import vitePluginString from 'vite-plugin-string';
import { fileURLToPath, URL } from 'url';

export default defineConfig({
  plugins: [vitePluginString(), solidPlugin()],
  server: {
    port: 3000
  },
  build: {
    target: 'esnext'
  },
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url)),
      '@editor': fileURLToPath(new URL('./src/editor', import.meta.url)),
      '@renderer': fileURLToPath(new URL('./src/editor/renderer', import.meta.url)),
      '@ui': fileURLToPath(new URL('./src/ui', import.meta.url)),
      '@icons': fileURLToPath(new URL('./src/ui/icons', import.meta.url)),
      '@inputs': fileURLToPath(new URL('./src/ui/inputs', import.meta.url)),
      '@menu': fileURLToPath(new URL('./src/ui/menu', import.meta.url)),
      '@navigation': fileURLToPath(new URL('./src/ui/navigation', import.meta.url)),
      '@multimedia': fileURLToPath(new URL('./src/ui/multimedia', import.meta.url)),
      '@math': fileURLToPath(new URL('./src/math', import.meta.url)),
      '@utils': fileURLToPath(new URL('./src/utils', import.meta.url))
    }
  }
});
