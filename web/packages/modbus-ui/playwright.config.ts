import { defineConfig, devices } from '@playwright/test';
export default defineConfig({
  testDir: './tests',
  timeout: 25 * 1000, // 30 seconds
  use: {
    headless: false,
    screenshot: 'on',
    video: 'off',
    isMobile: true,
    launchOptions: {
      args: ['--no-sandbox', '--disable-setuid-sandbox']
    },
    contextOptions: {
      ignoreHTTPSErrors: true
    },
    actionTimeout: 10000,
    navigationTimeout: 10000,
    viewport: { width: 1920, height: 1080 },
    ignoreHTTPSErrors: true,
    acceptDownloads: true,
    timezoneId: 'Europe/Paris',
    locale: 'en-US',
    permissions: ['clipboard-read', 'clipboard-write'],
    extraHTTPHeaders: {
      'Accept-Language': 'en-US,en;q=0.9',
      'Accept-Encoding': 'gzip, deflate, br',
      'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7',
    },
  },
  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'] },
    }
  ],
});