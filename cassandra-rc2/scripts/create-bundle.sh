#!/bin/bash

rimraf ./dist

## internal version
node scripts/version.js

## Python bundle
sh scripts/bundle-uploader.sh

# npm run build:clean
npm run build:release
npm web:update-release

## Copy to dist

cp ./.pio/build/waveshare-release/*.bin ./dist/
cp ./scripts/mklittlefs.exe ./dist/
cp ./scripts/esptool.exe ./dist/

## web

npm run web:clean && npm run web:build-dist && npm run web:sync 
cp -r ./data ./dist/data