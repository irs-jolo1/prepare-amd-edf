SUMMARY = "IRS base image"
DESCRIPTION = "Special-purpose Linux for ADC measurements by IRS used for the FEP project"
AUTHOR = "Joscha Loos (joscha.loos@irs.systems)"
LICENSE = "MIT"
CVE_PRODUCT = ""

inherit core-image

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
    kernel-modules \
    libiio-tests \
    stream-server-rs \
    ${CORE_IMAGE_EXTRA_INSTALL} \
"

IMAGE_LINGUAS = ""

IMAGE_ROOTFS_SIZE ?= "8192"

# set root users homedir to /root
EXTRA_USERS_PARAMS = "\
    usermod -d /root root; \
"

# TODO
# setenv bootargs "console=ttyPS0,115200 root=/dev/ram rw"
# saveenv
# or make sure its in device tree

# TODO
# setup image offsets
# CONFIG_SUBSYSTEM_UBOOT_DEVICETREE_OFFSET="0x100000"
# CONFIG_SUBSYSTEM_UBOOT_KERNEL_OFFSET=0x200000
# CONFIG_SUBSYSTEM_UBOOT_RAMDISK_IMAGE_OFFSET=0x4000000
# CONFIG_SUBSYSTEM_UBOOT_FIT_IMAGE_OFFSET=0x10000000
# CONFIG_SUBSYSTEM_UBOOT_BOOTSCR_OFFSET=0x20000000
