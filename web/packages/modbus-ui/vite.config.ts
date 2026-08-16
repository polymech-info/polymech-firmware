import { defineConfig } from "vite";
import react from "@vitejs/plugin-react-swc";
import path from "path";
import { visualizer } from "rollup-plugin-visualizer";
import viteCompression from 'vite-plugin-compression'
// https://vitejs.dev/config/
export default defineConfig(({ mode }) => ({
  server: {
    host: "::",
    port: 8080,
  },
  build: {
    outDir: "../../../cassandra-rc2/data/",
    sourcemap: mode === "development",
    rollupOptions: {
      output: {
        entryFileNames: 'assets/[name].js',
        chunkFileNames: 'assets/[name].js',
        assetFileNames: 'assets/[name].[ext]',
        manualChunks(id) {
          
          if (id.includes('node_modules')) {
            if (id.includes('lodash')) {
              return 'lodash';
            }
            if (id.includes('recharts')) {
              return 'recharts';
            }
            if (id.includes('zod')) {
              return 'zod';
            }
            return 'vendor';
          }
        },
      },
    },
  },
  plugins: [
    react(),
    visualizer({
      filename: "stats.html",
      open: false,
    }),
    viteCompression({ algorithm: 'gzip' })
  ],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./src"),
    },
  },
}));
