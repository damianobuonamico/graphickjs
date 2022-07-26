import { defineConfig } from 'vite';
import solidPlugin from 'vite-plugin-solid';
import { fileURLToPath, URL } from 'url';

export default defineConfig({
  plugins: [solidPlugin()],
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
      '@ui': fileURLToPath(new URL('./src/ui', import.meta.url)),
      '@icons': fileURLToPath(new URL('./src/ui/icons', import.meta.url)),
      '@inputs': fileURLToPath(new URL('./src/ui/inputs', import.meta.url)),
      '@menu': fileURLToPath(new URL('./src/ui/menu', import.meta.url)),
      '@navigation': fileURLToPath(
        new URL('./src/ui/navigation', import.meta.url)
      ),
      '@math': fileURLToPath(new URL('./src/math', import.meta.url)),
      '@utils': fileURLToPath(new URL('./src/utils', import.meta.url))
    }
  }
});
