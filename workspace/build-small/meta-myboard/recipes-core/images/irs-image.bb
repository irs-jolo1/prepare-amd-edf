SUMMARY = "IRS base image"
DESCRIPTION = "Special-purpose Linux for ADC measurements by IRS used for the FEP project"
AUTHOR = "Joscha Loos (joscha.loos@irs.systems)"
LICENSE = "MIT"
CVE_PRODUCT = ""

# This is the real rootfs, embedded directly into the kernel fitImage as a
# native FIT "ramdisk" component (see irs-distro.conf: INITRAMFS_IMAGE =
# "irs-image", INITRAMFS_IMAGE_BUNDLE = "0"). U-Boot loads the whole
# fitImage with a single `bootm`; the kernel's classic rd_load_image
# mechanism copies the embedded ramdisk into /dev/ram0 and mounts it
# directly as / (root=/dev/ram0 bootarg, see edf-linux-boot.cmd).
#
# ext4 (rather than cpio.gz) is used because it needs to be mountable as a
# normal block filesystem on /dev/ram0 - a cpio archive is not a mountable
# fs. FIT_SUPPORTED_INITRAMFS_FSTYPES in irs-distro.conf is set to "ext4"
# to match.
IMAGE_FSTYPES = "ext4"

# kernel-fitimage.bbclass looks for ${INITRAMFS_IMAGE}${IMAGE_MACHINE_SUFFIX}.ext4
# in DEPLOY_DIR_IMAGE (see FIT_SUPPORTED_INITRAMFS_FSTYPES), i.e. WITHOUT the
# default ".rootfs" IMAGE_NAME_SUFFIX. core-image-minimal-initramfs.bb does
# the same for the same reason - keep this in sync with it.
IMAGE_NAME_SUFFIX = ""

inherit core-image

# amd-edf-small.conf does:
#   ROOTFS_BOOTSTRAP_INSTALL:append = " ${MACHINE_ESSENTIAL_EXTRA_RDEPENDS}"
# which pulls kernel-image-fitimage etc back into every image's
# PACKAGE_INSTALL (image.bbclass). Since this image is embedded INTO the
# kernel FIT image, pulling the kernel image back into it here is circular
# and inflates it by tens of MB per rebuild.
#
# NOTE: a plain assignment (ROOTFS_BOOTSTRAP_INSTALL = "...") does NOT work
# to undo this - bitbake always applies :append operators (from any file,
# regardless of parse order) after plain assignments, so the distro's
# :append still wins. Only :remove is guaranteed to run last of all.
# PACKAGE_INSTALL:remove = "\
#     kernel-image-fitimage \
#     kernel-image-image.lzma \
#     kernel-image \
# "

# kernel-modules loads/installs kernel-modules
IMAGE_INSTALL = "\
    adc-aggregator \
    base-files \
    base-passwd \
    buildtime \
    busybox \
    busybox-mdev \
    dhcp \
    dns \
    homedir \
    initscripts \
    irsutils \
    stream-server-rs \
    ${CORE_IMAGE_EXTRA_INSTALL} \
"
# kernel-modules

IMAGE_LINGUAS = ""

IMAGE_ROOTFS_SIZE ?= "204800"
IMAGE_ROOTFS_EXTRA_SPACE = "20480"

# Must match (or be smaller than) CONFIG_BLK_DEV_RAM_SIZE (in KiB) in
# meta-myboard/recipes-kernel/linux/files/xilinx_defconfig, and the
# ramdisk_size= kernel bootarg in edf-linux-boot.cmd.

# set root users homedir to /root
EXTRA_USERS_PARAMS = "\
    usermod -d /root root; \
"



# TODO
# setup image offsets
# CONFIG_SUBSYSTEM_UBOOT_DEVICETREE_OFFSET="0x100000"
# CONFIG_SUBSYSTEM_UBOOT_KERNEL_OFFSET=0x200000
# CONFIG_SUBSYSTEM_UBOOT_RAMDISK_IMAGE_OFFSET=0x4000000
# CONFIG_SUBSYSTEM_UBOOT_FIT_IMAGE_OFFSET=0x10000000
# CONFIG_SUBSYSTEM_UBOOT_BOOTSCR_OFFSET=0x20000000

