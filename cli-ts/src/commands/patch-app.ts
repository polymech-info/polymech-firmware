import { promises as fs } from 'fs';
import path from 'path';
import * as cheerio from 'cheerio';

export const command = 'patch-app';
export const desc = 'Patch app by changing asset loading to be sequential.';

export const builder = (yargs) =>
  yargs.options({
    src: {
      describe: 'Path to source index.html',
      default: './data/index.html',
    },
    dst: {
      describe: 'Path to destination index.html',
      default: './data/index.html',
    },
  });

  const createLoaderScript = (assets) => {
    const assetsJson = JSON.stringify(assets, null, 2);
    return `
(function() {
  const assets = ${assetsJson};
  const DEFAULT_RETRIES = 5;
  const RETRY_DELAY_MS = 1500;
  const SCRIPT_SUCCESS_DELAY_MS = 800;
  const NEXT_ASSET_DELAY_MS = 1500;

  let assetIndex = 0;
  let allAssetsLoaded = false;
  let appIsReady = false;
  const splashText = document.getElementById('loading-splash-text');

  function updateSplash(text) {
    if (splashText) {
      splashText.textContent = text;
    }
  }

  function hideSplash() {
    const splash = document.getElementById('loading-splash');
    if (splash) {
      splash.style.opacity = '0';
      setTimeout(() => splash.remove(), 500); // fade out
    }
  }

  function checkAndHideSplash() {
    if (allAssetsLoaded && appIsReady) {
      hideSplash();
    }
  }

  window.markAppAsReady = function() {
    console.log('markAppAsReady called.');
    appIsReady = true;
    checkAndHideSplash();
  };

  function onAllAssetsLoaded() {
    console.log('All assets loaded sequentially.');
    window.dispatchEvent(new CustomEvent('allAssetsLoaded'));
    allAssetsLoaded = true;
    checkAndHideSplash();
  }

  function loadNextAsset() {
    if (assetIndex >= assets.length) {
      onAllAssetsLoaded();
      return;
    }

    const asset = assets[assetIndex];
    const assetUrl = asset.attrs.href || asset.attrs.src;
    const isCss = assetUrl && assetUrl.endsWith('.css');
    
    updateSplash('Loading: ' + (assetUrl || 'asset...'));

    if (!assetUrl) {
      console.error('Asset has no href or src', asset);
      assetIndex++;
      const delay = appIsReady ? 1500 : NEXT_ASSET_DELAY_MS;
      setTimeout(loadNextAsset, delay);
      return;
    }

    if (isCss) {
      // Use fetch to inline CSS
      function fetchWithRetry(url, retries = DEFAULT_RETRIES, retryDelay = RETRY_DELAY_MS) {
        updateSplash('Fetching: ' + url);
        fetch(url)
          .then(response => {
            if (response.ok) return response.text();
            if (response.status === 503 && retries > 0) {
              updateSplash('Error 503, retrying: ' + url + ' (' + (retries - 1) + ' left)');
              console.warn('Request for ' + url + ' failed with 503. Retrying in ' + retryDelay + 'ms... (' + retries + ' retries left)');
              setTimeout(() => fetchWithRetry(url, retries - 1, retryDelay), retryDelay);
              return null;
            }
            throw new Error('Request failed with status ' + response.status);
          })
          .then(text => {
            if (text === null) return; // Retry in progress

            updateSplash('Inlining CSS: ' + url);
            console.log('Successfully inlined CSS from:', url);
            const style = document.createElement('style');
            style.textContent = text;
            document.head.appendChild(style);

            assetIndex++;
            const delay = appIsReady ? 0 : NEXT_ASSET_DELAY_MS;
            setTimeout(loadNextAsset, delay);
          })
          .catch(err => {
            updateSplash('Failed to load CSS: ' + url);
            console.error('Failed to load CSS asset ' + url + ' after retries:', err);
            assetIndex++;
            const delay = appIsReady ? 0 : NEXT_ASSET_DELAY_MS;
            setTimeout(loadNextAsset, delay);
          });
      }
      fetchWithRetry(assetUrl);
    } else {
      // Use tag-based loading for JS
      let retries = DEFAULT_RETRIES;
      const retryDelay = RETRY_DELAY_MS;

      function attemptLoad() {
        updateSplash('Loading script: ' + (assetUrl || '...'));
        const element = document.createElement(asset.tagName);
        for (const attr in asset.attrs) {
          element.setAttribute(attr, asset.attrs[attr]);
        }

        element.onload = () => {
          updateSplash('Loaded script: ' + assetUrl);
          console.log('Successfully loaded asset:', assetUrl);
          assetIndex++;
          const delay = appIsReady ? 0 : SCRIPT_SUCCESS_DELAY_MS;
          setTimeout(loadNextAsset, delay);
        };

        element.onerror = () => {
          element.remove();
          console.warn('Failed to load asset:', assetUrl);
          if (retries > 0) {
            retries--;
            updateSplash('Retrying script: ' + assetUrl + ' (' + retries + ' left)');
            console.log('Retrying... (' + retries + ' retries left)');
            setTimeout(attemptLoad, retryDelay);
          } else {
            updateSplash('Failed to load script: ' + assetUrl);
            console.error('Failed to load asset after multiple retries:', assetUrl);
            assetIndex++;
            const delay = appIsReady ? 0 : NEXT_ASSET_DELAY_MS;
            setTimeout(loadNextAsset, delay);
          }
        };

        document.head.appendChild(element);
      }
      attemptLoad();
    }
  }

  loadNextAsset();
})();
`;
  };


export const handler = async (argv) => {
  const { src, dst } = argv;

  try {
    const srcPath = path.resolve(src);
    const dstPath = path.resolve(dst);
    
    console.log(`Patching ${srcPath} for sequential asset loading...`);

    const html = await fs.readFile(srcPath, 'utf-8');
    const $ = cheerio.load(html);

    const splashCss = `
#loading-splash {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: #222;
  color: #eee;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  z-index: 9999;
  font-family: "Courier New", Courier, monospace;
  font-size: 1em;
  text-align: center;
  transition: opacity 0.5s ease-out;
}
#loading-splash-text {
  margin-top: 20px;
  min-height: 1.2em;
}
`;
    const splashHtml = `
<div id="loading-splash">
  <h3>Cassandra RC2</h3>
  <p>Initializing Application...</p>
  <div id="loading-splash-text"></div>
</div>
`;

    $('head').append(`<style>${splashCss}</style>`);
    $('body').prepend(splashHtml);

    const assetsToLoad: { tagName: string; attrs: { [name: string]: string; } }[] = [];

    $('head link[href^="/assets/"], head script[src^="/assets/"]').each(function () {
      const el = $(this);
      const tagName = el.prop('tagName');
      if (!tagName) return;

      const attrs = el.attr();

      if (attrs && (attrs.src || attrs.href)) {
        assetsToLoad.push({
          tagName: tagName.toLowerCase(),
          attrs,
        });
        el.remove();
      }
    });

    if (assetsToLoad.length > 0) {
      const loaderScriptContent = createLoaderScript(assetsToLoad);
      $('body').append(`<script>${loaderScriptContent}</script>`);
      console.log(`Injected sequential asset loader script with ${assetsToLoad.length} assets.`);
    } else {
      console.log('No assets found to patch in <head>.');
    }
    
    await fs.writeFile(dstPath, $.html());
    console.log(`Successfully patched and saved to ${dstPath}`);
  } catch (error) {
    console.error('Error patching app:', error);
    process.exit(1);
  }
}; 