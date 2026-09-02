MACHINE="adaq7768"

echo "Removing old files"
rm /media/joscha/ZYNQMP/*

echo "Copying new files"
cp -L tmp/deploy/images/${MACHINE}/boot.bin /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/system.dtb /media/joscha/ZYNQMP

# IMPORTANT: do NOT use the plain "fitImage" symlink here - with
# INITRAMFS_IMAGE_BUNDLE="0" (see irs-distro.conf), kernel-fitimage.bbclass
# never re-points it at the variant containing the embedded ramdisk; it
# stays pointed at "fitImage-none" (kernel+fdt only, no rootfs at all).
# "fitImage-${INITRAMFS_IMAGE_NAME}-${KERNEL_FIT_LINK_NAME}" is the stable
# symlink for the variant that actually embeds irs-image as a FIT ramdisk
# node - that's the one edf-linux-boot.cmd/bootm needs.
cp -L tmp/deploy/images/${MACHINE}/fitImage-irs-image-${MACHINE}-${MACHINE} /media/joscha/ZYNQMP/fitImage

cp -L tmp/deploy/images/${MACHINE}/boot.scr /media/joscha/ZYNQMP

echo "Syncing"
sync

