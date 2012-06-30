#!/bin/bash

cp flus_driver/onda.ko output/target/root/
make
sudo mke2fs -j /dev/sda2
sudo mount -t ext3 /dev/sda2 /mnt/
sudo tar xvf output/images/rootfs.tar -C /mnt/
sudo umount /mnt/
