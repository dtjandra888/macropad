import { svelte } from '@sveltejs/vite-plugin-svelte'
import { defineConfig } from 'vite'
import { svelteTesting } from "@testing-library/svelte/vite";

export default defineConfig({
  plugins: [
      svelte(),
      svelteTesting(),
  ],
  server: {
    proxy: {
      "/api": {
        target: "http://localhost:3000",
        changeOrigin: true,
      },
    },
  },
  base: "./",
  test: {
    environment: "jsdom", 
  },
});
