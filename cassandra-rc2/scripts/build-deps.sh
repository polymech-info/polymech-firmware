#!/bin/bash

node scripts/version.js

cd ../web/packages/client
npm run build

#cd ../modbus-ui
#npm run build

cd ../../../cli-ts
sh scripts/build.sh
