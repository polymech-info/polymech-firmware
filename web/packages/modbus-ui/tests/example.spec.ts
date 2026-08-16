import { test, expect } from '@playwright/test'
const prod = false
const LANG = 'en'

const TEST_URL = prod ? 'http://localhost:8080/' : 'http://192.168.1.250/'

// --- Screenshot Flags ---
const screenshotFullPages = true;
const screenshotSignalPlot = true;
const screenshotSequentialHeating = true;
const screenshotControllerChart = true;
const screenshotSettings = true;
const screenshotNetworkSettings = true;
const screenshotProfiles = true;
const screenshotCoils = true;
const screenshotRegisters = true;
const screenshotLogs = true;
const screenshotCharts = true;

const paths = [
  '#/',
  '#/advanced/settings',
  '#/advanced/network',
  '#/signals',
  '#/profiles',
  '#/advanced/coils',
  '#/advanced/registers',
  '#/advanced/logs',
  '#/advanced/charts',
];

test('Take screenshots', async ({ page }) => {
  test.setTimeout(300000); // 5 minutes
  page.setDefaultNavigationTimeout(120000); // 2 minutes
  console.log('TEST_URL', TEST_URL);

  if (screenshotFullPages) {
    console.log('--- Taking Full Page Screenshots ---');
    for (const path of paths) {
      const url = `${TEST_URL}${path}`;
      const screenshotName = path.replace(/#\//g, '').replace('/', '') || 'root';
      const screenshotPath = `./tests/screenshot-${screenshotName}.jpg`;

      console.log(`Navigating to ${url} and taking screenshot ${screenshotPath}`);

      await page.goto(url, { waitUntil: 'networkidle' });
      await page.waitForTimeout(18000);

      await page.screenshot({ path: screenshotPath, fullPage: true, quality: 100 });
    }
  }

  // --- Component Screenshots ---

  // Debug: Take a full page screenshot to see what's rendered
  console.log('--- Taking debug screenshot of home page ---');
  await page.goto(TEST_URL, { waitUntil: 'networkidle' });
  await page.waitForTimeout(8000);
  await page.screenshot({ path: './tests/debug-home-page.jpg', fullPage: true, quality: 100 });

  // Start a profile before taking screenshots
  console.log('--- Starting profile for testing ---');
  await page.goto(TEST_URL, { waitUntil: 'networkidle' });
  await page.waitForTimeout(8000);

  // Look for the play button in the playback controls
  const playButton = page.locator('#playback-controls-container button').first();
  if (await playButton.count() > 0) {
    console.log('Clicking play button to start profile...');
    await playButton.click();
    await page.waitForTimeout(8000); // Wait for profile to start
  } else {
    console.log('Play button not found, continuing without starting profile...');
  }

  if (screenshotSignalPlot) {
    // Signal Plot Editor
    console.log('--- Taking screenshot of Signal Plot Editor ---');
    await page.goto(`${TEST_URL}#/signals`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(3000); // Wait for data to load and components to render

    // Ensure a control point is selected to show properties
    const firstControlPoint = await page.locator('[data-cp-id]').first();
    if (await firstControlPoint.count() > 0) {
      console.log('Selecting a control point...');
      await firstControlPoint.click();
      await page.waitForTimeout(500); // Wait for properties panel to update
    } else {
      console.log('No control points found to select.');
    }

    const signalPlotElement = await page.locator('#signal-plot-collapsible-0').first();
    if (await signalPlotElement.count() > 0) {
      console.log('Taking screenshot of #signal-plot-collapsible-0');
      await signalPlotElement.screenshot({ path: './tests/screenshot-signal-plot-editor.jpg', quality: 100 });
    } else {
      console.log('Could not find #signal-plot-collapsible-0 element to screenshot.');
    }
  }

  if (screenshotSequentialHeating) {
    // Sequential Heating Card
    console.log('--- Taking screenshot of Sequential Heating Card ---');
    await page.goto(TEST_URL, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);

    const sequentialHeatingElement = page.locator('#sequential-heating-card');

    // Check if the card is visible. If not, try to open the collapsible section.
    if (!await sequentialHeatingElement.isVisible()) {
      console.log('Sequential heating card not visible, attempting to expand section...');
      const sequentialHeatingCollapsible = page.locator('#hmi-sequential-heating-collapsible');
      console.log(`Found ${await sequentialHeatingCollapsible.count()} collapsible elements`);
      if (await sequentialHeatingCollapsible.count() > 0) {
        console.log('Clicking on sequential heating collapsible...');
        await sequentialHeatingCollapsible.click();
        await page.waitForTimeout(3000); // Wait for animation
        console.log(`After click, sequential heating card visible: ${await sequentialHeatingElement.isVisible()}`);
      }
    }

    if (await sequentialHeatingElement.count() > 0) {
      console.log('Taking screenshot of #sequential-heating-card');
      await sequentialHeatingElement.screenshot({ path: './tests/screenshot-sequential-heating.jpg', quality: 100 });
    } else {
      console.log('Could not find #sequential-heating-card element to screenshot.');
    }
  }

  if (screenshotControllerChart) {
    // Controller Chart
    console.log('--- Taking screenshot of Controller Chart ---');
    await page.goto(TEST_URL, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);

    const controllerChartElement = page.locator('#controller-chart');

    if (!await controllerChartElement.isVisible()) {
      console.log('Controller chart not visible, attempting to expand section...');
      const controllerChartCollapsible = page.locator('#hmi-chart-collapsible');
      console.log(`Found ${await controllerChartCollapsible.count()} chart collapsible elements`);
      if (await controllerChartCollapsible.count() > 0) {
        console.log('Clicking on controller chart collapsible...');
        await controllerChartCollapsible.click();
        await page.waitForTimeout(1000);
        console.log(`After click, controller chart visible: ${await controllerChartElement.isVisible()}`);
      }
    }

    if (await controllerChartElement.count() > 0) {
      console.log('Taking screenshot of #controller-chart');
      await controllerChartElement.screenshot({ path: './tests/screenshot-controller-chart.jpg', quality: 100 });
    } else {
      console.log('Could not find #controller-chart element to screenshot.');
    }
  }

  if (screenshotSettings) {
    // Settings
    console.log('--- Taking screenshot of Settings Page ---');
    await page.goto(`${TEST_URL}#/advanced/settings`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);

    const settingsElement = await page.locator('#cassandra-settings-display').first();
    if (await settingsElement.count() > 0) {
      console.log('Taking screenshot of #cassandra-settings-display');
      await settingsElement.screenshot({ path: './tests/screenshot-settings.jpg', quality: 100 });
    } else {
      console.log('Could not find #cassandra-settings-display element to screenshot.');
    }
  }

  if (screenshotNetworkSettings) {
    // Network Settings
    console.log('--- Taking screenshot of Network Settings Page ---');
    await page.goto(`${TEST_URL}#/advanced/network`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);

    const networkSettingsElement = await page.locator('#network-settings-display').first();
    if (await networkSettingsElement.count() > 0) {
      console.log('Taking screenshot of #network-settings-display');
      await networkSettingsElement.screenshot({ path: './tests/screenshot-network-settings.jpg', quality: 100 });
    } else {
      console.log('Could not find #network-settings-display element to screenshot.');
    }
  }

  if (screenshotProfiles) {
    // Profiles
    console.log('--- Taking screenshot of Profiles Page ---');
    await page.goto(`${TEST_URL}#/profiles`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);
    console.log('Taking screenshot of #profiles-display');
    await page.locator('#profiles-display').screenshot({ path: './tests/screenshot-profiles.jpg', quality: 100 });
  }

  if (screenshotCoils) {
    // Coils
    console.log('--- Taking screenshot of Coils Page ---');
    await page.goto(`${TEST_URL}#/advanced/coils`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);
    console.log('Taking screenshot of #coils-display');
    await page.locator('#coils-display').screenshot({ path: './tests/screenshot-coils.jpg', quality: 100 });
  }

  if (screenshotRegisters) {
    // Registers
    console.log('--- Taking screenshot of Registers Page ---');
    await page.goto(`${TEST_URL}#/advanced/registers`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);
    console.log('Taking screenshot of #registers-display');
    await page.locator('#registers-display').screenshot({ path: './tests/screenshot-registers.jpg', quality: 100 });
  }

  if (screenshotLogs) {
    // Logs
    console.log('--- Taking screenshot of Logs Page ---');
    await page.goto(`${TEST_URL}#/advanced/logs`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);
    console.log('Taking screenshot of #logs-display');
    await page.locator('#logs-display').screenshot({ path: './tests/screenshot-logs.jpg', quality: 100 });
  }

  if (screenshotCharts) {
    // Charts
    console.log('--- Taking screenshot of Charts Page ---');
    await page.goto(`${TEST_URL}#/advanced/charts`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(8000);
    console.log('Taking screenshot of #charts-display');
    await page.locator('#charts-display').screenshot({ path: './tests/screenshot-charts.jpg', quality: 100 });
  }
});
