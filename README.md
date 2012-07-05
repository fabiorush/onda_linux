onda_linux
==========

mmc rescan 0; fatload mmc 0 0x82000000 uImage; fatload mmc 0 0x83000000 rootfs.squashfs
setenv bootargs console=ttyO2,115200n8 mpurate=1000 root=/dev/ram rw initrd=0x83000000,3858432 rootfstype=squashfs; bootm 0x82000000

