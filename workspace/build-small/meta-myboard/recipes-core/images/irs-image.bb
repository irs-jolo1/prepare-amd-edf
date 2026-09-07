SUMMARY = "IRS base image"
DESCRIPTION = "Special-purpose Linux for ADC measurements by IRS used for the FEP project"
AUTHOR = "Joscha Loos (joscha.loos@irs.systems)"
LICENSE = "MIT"
CVE_PRODUCT = ""

# Journaling and reserved-root-blocks only matter for persistent storage
# that survives power loss / needs crash recovery, or supports growing
# free space over time. This is a RAM disk, rebuilt from scratch on every
# boot - both are pure overhead here.
EXTRA_IMAGECMD:ext4 = "-i 4096 -O ^has_journal -m 0"

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
    stream-server-rs \
    irsutils \
    ${CORE_IMAGE_EXTRA_INSTALL} \
"
# kernel-modules

IMAGE_LINGUAS = ""

# These act as a FLOOR, not a fixed size: get_rootfs_size() in
# image.bbclass uses max(actual_content_size * IMAGE_OVERHEAD_FACTOR,
# IMAGE_ROOTFS_SIZE) + IMAGE_ROOTFS_EXTRA_SPACE. Keep these small - this is
# a RAM disk, not growable storage, so there's no need to pad it beyond a
# small safety margin for future package growth.
IMAGE_ROOTFS_SIZE ?= "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "8192"

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

