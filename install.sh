#!/bin/bash

rm output/build/linux-3.4.2/{.stamp_installed,.stamp_compiled}
make
sudo mount -t vfat /dev/sda1 /mnt/
sudo cp output/images/uImage /mnt/
sudo umount /mnt/
