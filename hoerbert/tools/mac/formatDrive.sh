#!/bin/sh
EJECTABLE=`diskutil info "$1" | grep 'Ejectable' | cut -d : -f 2 | sed 's/^ *//g' | sed 's/ *$//g';`
REMOVABLE=`diskutil info "$1" | grep 'Removable Media' | cut -d : -f 2 | sed 's/^ *//g' | sed 's/ *$//g';`
NODE=`diskutil info "$1" | grep 'Device Node' | cut -d : -f 2 | sed 's/^ *//g' | sed 's/ *$//g';`
if [ "$EJECTABLE" != "Yes" ] && [ "$REMOVABLE" != "Yes" ] && [ "$REMOVABLE" != "Removable" ]; then
  exit 1
fi
logger User formatting sd card with hörbert software: /usr/sbin/diskutil eraseVolume "MS-DOS FAT32" HOERBERT "$NODE" 
/usr/sbin/diskutil eraseVolume "MS-DOS FAT32" HOERBERT "$NODE"