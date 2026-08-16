#!/bin/bash

# Example script for Google Drive operations using rclone

# Define local and remote paths
LOCAL_DIR="./.pio/build/waveshare-release"
GDRIVE_DIR="polymech:/httpdocs/cassandra-rc2"

rclone copy "${LOCAL_DIR}/" "${GDRIVE_DIR}" --progress --transfers 4 --include "*.bin" --verbose
