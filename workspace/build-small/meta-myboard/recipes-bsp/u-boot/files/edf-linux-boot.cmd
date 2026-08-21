setenv bootargs "earlycon console=ttyPS0,115200 systemd.log_target=console"

fatload mmc 1:1 ${kernel_addr_r} Image.lzma
fatload mmc 1:1 ${fdt_addr_r} system.dtb
fatload mmc 1:1 ${ramdisk_addr_r} cpio.gz.u-boot

booti ${kernel_addr_r} ${ramdisk_addr_r}:${ramdisk_size} ${fdt_addr_r}

