SUMMARY = "Custom IRS FEP Kernel"
DESCRIPTION = "Mainline Linux with custom patches"
AUTHOR = "joscha loos (joscha.loos@irs.systems)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# which CVE product to target
CVE_PRODUCT = "linux_kernel"

# inherit kernel externalsrc
inherit kernel kernel-yocto externalsrc

SRC_URI = "\
    file://kconfig.cfg \
    file://xilinx_defconfig \
"

COMPATIBLE_MACHINE = "adaq7768"

EXTERNALSRC = "/linux"
