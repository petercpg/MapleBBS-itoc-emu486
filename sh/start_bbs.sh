#!/bin/sh
#
# MapleBBS Startup Script
# Replicates rc.local logic for Systemd
#

echo '/home/bbs/bin/bbsd'
/home/bbs/bin/bbsd

echo '/home/bbs/bin/bmtad'
/home/bbs/bin/bmtad

echo '/home/bbs/bin/gemd'
/home/bbs/bin/gemd

echo '/home/bbs/bin/bguard'
/home/bbs/bin/bguard

echo '/home/bbs/bin/bpop3d'
/home/bbs/bin/bpop3d

echo '/home/bbs/bin/bnntpd'
/home/bbs/bin/bnntpd

echo '/home/bbs/bin/xchatd'
/home/bbs/bin/xchatd

echo '/home/bbs/innd/innbbsd'
/home/bbs/innd/innbbsd

echo '/home/bbs/bin/camera'
su bbs -c '/home/bbs/bin/camera'

echo '/home/bbs/bin/account'
su bbs -c '/home/bbs/bin/account'
