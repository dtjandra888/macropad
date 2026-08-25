#!/bin/bash

echo "Starting transfer to rp2040..."

sudo umount /mnt/usb
sudo mount /dev/sda1 /mnt/usb
sudo cp build/macro.uf2 /mnt/usb

echo "Done!"
