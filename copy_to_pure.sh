#!/bin/bash 
set -euo pipefail

if [[ $1 ]]; then 
    SD="$1"
else
    echo "provide blockdev ie: /dev/sdc"
    exit 1
fi

make -C build
cp build/updater/PureUpdater_RT.bin /run/media/pholat/MUDITAOS/current/boot.bin
udisksctl unmount -b "$SD"1
udisksctl unmount -b "$SD"2
sudo eject "$SD"

