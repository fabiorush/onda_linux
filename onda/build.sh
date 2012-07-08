#!/bin/bash

PATH="$PWD/../output/host/bin:/home/flus/buildroot/output/host/usr/bin:$PWD/../output/host/usr/sbin/:/usr/bin:/bin:/opt/bin:/usr/i686-pc-linux-gnu/gcc-bin/4.5.3" PERLLIB="$PWD/../output/host/usr/lib/perl" /usr/bin/make -j4 HOSTCC="/usr/bin/gcc" HOSTCFLAGS="" ARCH=arm INSTALL_MOD_PATH=$PWD/../output/target CROSS_COMPILE=" $PWD/../output/host/usr/bin/arm-none-linux-gnueabi-" LZMA="$PWD/../output/host/usr/bin/lzma" -C $PWD/../output/build/linux-3.4.4 M=$PWD modules && \
cp onda.ko ../output/target/root/
