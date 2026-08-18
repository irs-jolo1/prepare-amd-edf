DESCRIPTION = "Minimal console image"

IMAGE_INSTALL= "\
 base-files \
 base-passwd \
 busybox \
 initscripts \
 libiio-tests \
 ${CORE_IMAGE_EXTRA_INSTALL} \
"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

IMAGE_ROOTFS_SIZE ?= "8192"

# disable unused features
DISTRO_FEATURES:remove = "\
    3g \
    alsa \
    bluetooth \
    debuginfod \
    fbdev \
    gobject-introspection-data \
    ipv6 \
    multiarch \
    nfc \
    nfs \
    openamp \
    opengl \
    pam \
    pci \
    pcmcia \
    ptest \
    pulseaudio \
    security \
    tpm \
    usbgadget \
    vfat \
    virtualization \
    libmali \
    systemd \
    ldconfig \
    vmsep \
    wayland \
    wifi \
    x11 \
    xen \
    zeroconf \
"

# replace systemd with sysvinit
# DISTRO_FEATURES:append = " sysvinit"
# VIRTUAL-RUNTIME_init_manager = "sysvinit"
# VIRTUAL-RUNTIME_initscripts = "initscripts"


# setup which rootfs types are built
IMAGE_FSTYPES = "cpio.gz.u-boot"
# IMAGE_FSTYPES:append = " wic"'

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
