#!/bin/bash
cp -r ./config/* ./data/
rm ./data/assets/*.js
rm ./data/assets/*.css
pm-fw-cli patch-app --src=./data/index.html --dst ./data/index.html

#rm ./data/assets/*.gz
#rm ./data/assets/*.woff
#rm ./data/assets/*.woff2
#rm ./data/assets/*.ttf
#rm ./data/assets/*.js
