#!/bin/bash
set -e

# Default environment
ENV="waveshare"

# Check for environment override if the first argument does not look like an address
if [ ! -z "$1" ] && [[ ! $1 =~ ^0x.* ]]; then
    ENV=$1
    shift
fi

ADDRESSES=$@
ELF_FILE=".pio/build/$ENV/firmware.elf"

if [ $# -eq 0 ]; then
    echo "Usage: ./scripts/decode.sh [env] <addr1> <addr2> ..."
    echo "Example: ./scripts/decode.sh waveshare 0x40377a9e 0x4037cee9"
    exit 1
fi

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: ELF file not found: $ELF_FILE"
    echo "Build the project for environment '$ENV' first."
    exit 1
fi

echo "Decoding for environment: $ENV"
echo "ELF file: $ELF_FILE"
echo "Addresses: $ADDRESSES"
echo ""

pio pkg exec -p "toolchain-xtensa-esp32s3" -- xtensa-esp32s3-elf-addr2line -pfiaC -e $ELF_FILE $ADDRESSES 