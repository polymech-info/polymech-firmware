#!/bin/bash

# Define local and remote paths
LOCAL_DIR="./clients/plastiq"
GDRIVE_DIR="polymech.info:/httpdocs/plastichub"

rclone copy "${LOCAL_DIR}/" "${GDRIVE_DIR}" --progress --transfers 4 --include "*.zip" --verbose

# //http://polymech.info/plastichub/master.zip