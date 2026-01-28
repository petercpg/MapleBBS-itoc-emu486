#!/bin/sh
# Using ps instead of top for portable machine-parsable process identification
# Find processes named bbsd that are consuming CPU (simplified approach)
# Safety: ps -u bbs ensures we only list processes owned by 'bbs' user
kill -9 `ps -u bbs -o pid,comm | grep bbsd | awk '{print $1}'` 2>/dev/null
