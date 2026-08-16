import { test, expect } from '@playwright/test'
const prod = false
const LANG = 'en'
const STORE = `/${LANG}/store/`
const TEST_URL = 'http://192.168.1.250/'
const ITEM_URL = (item) => `${TEST_URL}/${item}`

test('Take a primary app screenshot', async ({ page }) => {
  await page.goto(TEST_URL, { waitUntil: 'networkidle' });
  
  await new Promise((resolve) => setTimeout(resolve, 2250));
  const viewportSize = page.viewportSize();
  if (viewportSize) {
    await page.setViewportSize(viewportSize);
    await page.waitForTimeout(500);
  }
  await page.screenshot({ path: './tests/screenshot-latest.jpg', fullPage: true, quality: 100 });
});


test('Take a dashboard screenshot', async ({ page }) => {
  await page.goto(TEST_URL + '#/dashboard', { waitUntil: 'networkidle' });
  await new Promise((resolve) => setTimeout(resolve, 2250));
  const viewportSize = page.viewportSize();
  if (viewportSize) {
    await page.setViewportSize(viewportSize);
    await page.waitForTimeout(500);
  }

  await page.screenshot({ path: './tests/screenshot-modbus.jpg', fullPage: true, quality: 100 });
});
