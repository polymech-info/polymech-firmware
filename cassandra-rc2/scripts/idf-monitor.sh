#!/bin/bash
echo "Setting up ESP-IDF environment..."
# shellcheck disable=SC1091
. /c/Espressif/frameworks/esp-idf-v5.3.1/export.sh
echo "ESP-IDF environment set up. Starting monitor..."
idf.py monitor 