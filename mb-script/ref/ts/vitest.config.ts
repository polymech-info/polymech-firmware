import { defineConfig } from 'vitest/config';

export default defineConfig({
  test: {
    globals: true, // Make Vitest's APIs (describe, it, expect, etc.) available globally
    environment: 'node', // Specify the test environment
    coverage: {
      provider: 'v8', // or 'istanbul'
      reporter: ['text', 'json', 'html'],
    },
  },
}); 