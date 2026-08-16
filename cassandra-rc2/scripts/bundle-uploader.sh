#!/bin/bash
#
# Creates a standalone Windows executable for the uploader utility.
#
# This script uses PyInstaller to bundle the 'upload_littlefs.py' script and its
# dependencies into a single .exe file. This allows running the uploader on
# Windows without a full Python environment.
#
# It bundles 'esptool.exe' and 'mklittlefs.exe' directly into the executable.
#
# PREREQUISITES:
#   - Python must be installed and accessible from the shell.
#   - PyInstaller must be installed (`pip install pyinstaller`).
#   - 'mklittlefs.exe' and 'esptool.exe' must be placed in the 'scripts/' directory.
#
# USAGE:
#   1. Run this script: ./scripts/bundle-uploader.sh
#   2. Place firmware files into the generated 'dist/' directory.
#   3. Run 'dist/cassandra-uploader.exe' to upload.

set -e

echo "--- Bundling Full Uploader for Windows ---"

# --- Configuration ---
TARGET_SCRIPT="scripts/upload_littlefs.py"
EXE_NAME="cassandra-uploader"
# List of binaries to bundle. Assumes they are in the 'scripts' folder.
# The ':.' part places them in the root of the bundle's temporary directory.
BINARIES_TO_ADD=(
    "scripts/mklittlefs.exe:."
    "scripts/esptool.exe:."
)

# --- Pre-flight Checks ---
if ! command -v pyinstaller &> /dev/null; then
    echo "❌ Error: PyInstaller is not installed or not in your PATH."
    echo "Please install it by running: pip install pyinstaller"
    exit 1
fi

for binary_path in "${BINARIES_TO_ADD[@]}"; do
    # Strip the ':.' suffix for the check
    actual_path="${binary_path%:.*}"
    if [ ! -f "$actual_path" ]; then
        echo "❌ Error: Required binary '$actual_path' was not found."
        echo "Please ensure it is in the 'scripts' directory."
        exit 1
    fi
done

echo "✅ Pre-flight checks passed. Starting PyInstaller..."

# --- Build PyInstaller Command ---
PYINSTALLER_CMD=(
    "pyinstaller"
    "--noconfirm"
    "--onefile"
    "--name" "$EXE_NAME"
    # Add hidden imports for pyserial to ensure port detection works in the bundle.
    "--hidden-import=serial.tools.list_ports"
    "--hidden-import=serial.tools.list_ports_windows"
)

for binary in "${BINARIES_TO_ADD[@]}"; do
    PYINSTALLER_CMD+=("--add-binary" "$binary")
done

PYINSTALLER_CMD+=("$TARGET_SCRIPT")

# --- Run PyInstaller ---
echo "Running command: ${PYINSTALLER_CMD[*]}"
"${PYINSTALLER_CMD[@]}"

echo ""
echo "---"
echo "✅ Success! Bundled executable created at: dist/$EXE_NAME.exe"
echo ""
echo "➡️ NEXT STEPS:"
echo "   1. Copy your firmware files ('firmware.bin', etc.) into the 'dist/' folder."
echo "   2. Ensure 'pm-fw-cli.exe' and 'assets/network.json' are present in 'dist/' for --restore feature."
echo "   3. Run 'dist/$EXE_NAME.exe' to upload everything to your device."
echo "---"
