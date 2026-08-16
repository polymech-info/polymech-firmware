import { defineConfig } from 'vitest/config';

export default defineConfig({
  // Disable multi-threading to run tests sequentially
  threads: false,
  test: {
    // Explicitly include test files in the tests directory
    include: ['tests/**/*.test.ts'],
    // Optional: Increase default test timeout if needed for hardware tests
    testTimeout: 10000, // 10 seconds
    // Run tests sequentially
    sequence: {
      concurrent: false
    },
  },
}); 