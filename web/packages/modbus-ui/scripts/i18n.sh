osr-i18n translate \
--srcLang='EN' \
--dstLang='NL,ES,FR,IT,DE' \
--debug \
--logLevel='debug' \
--createGlossary=false \
--src='./src/i18n/en.json' \
--dst='./src/i18n/${DST_LANG}${SRC_EXT}'
