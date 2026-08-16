#!/bin/bash

# Example script for Google Drive operations using rclone

# Define local and remote paths
LOCAL_DIR="./clients/mesint"
GDRIVE_DIR="polymech.info:/httpdocs/mesint"

rclone copy "${LOCAL_DIR}/" "${GDRIVE_DIR}" --progress --transfers 4 --include "*.zip" --verbose

# //http://polymech.info/mesint/master.zip | http://polymech.info/mesint/slave.zip
