# Single-file FIT boot:
#  fitImage embeds kernel + fdt + irs-image (the real rootfs, built as
#  ext4) as a native FIT "ramdisk" component (see irs-distro.conf:
#  INITRAMFS_IMAGE = "irs-image", INITRAMFS_IMAGE_BUNDLE = "0").
#  U-Boot's `bootm` picks kernel+fdt+ramdisk from the FIT's default
#  configuration automatically - no separate fatload of the rootfs needed.
#  The kernel's classic rd_load_image mechanism then copies the embedded
#  ramdisk into the /dev/ram0 block device (CONFIG_BLK_DEV_RAM, see
#  xilinx_defconfig) and mounts it directly as / (root=/dev/ram0 below) -
#  same as booting from any other block device, no initramfs involved.
#
# ramdisk_size (KiB) must be >= the actual irs-image ext4 file size, and
# <= CONFIG_BLK_DEV_RAM_SIZE in xilinx_defconfig (currently 245760 KiB).
setenv bootargs "earlycon console=ttyPS0,115200 root=/dev/ram0 rw ramdisk_size=245760"

fatload mmc 1:1 ${kernel_addr_r} fitImage

bootm ${kernel_addr_r}

