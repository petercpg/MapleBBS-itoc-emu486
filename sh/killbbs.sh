#!/bin/sh
# 清除站上使用者與shared memory
# Use portable ps flags common to both Linux and BSD
# Kill the specific daemons first, then the main bbsd processes
# List of daemons to kill: bbsd bmtad gemd bguard bpop3d bnntpd xchatd innbbsd
DAEMONS="bbsd|bmtad|gemd|bguard|bpop3d|bnntpd|xchatd|innbbsd"
kill `ps -ax | grep -E "($DAEMONS)" | grep -v grep | awk '{print $1}'` 2>/dev/null

# Identify OS
OS=`uname -s`

# Standardize IPC removal logic
# Linux/FreeBSD support ipcs -m and ipcrm -m/-M
for i in `ipcs -m | grep bbs | awk '{print $2}'`
do
    if [ "$OS" = "FreeBSD" ]; then
        ipcrm -m "$i"
    elif [ "$OS" = "Linux" ]; then
        ipcrm -m "$i"
    fi
done
