SUMMARY = "Custom IRS FEP Kernel"
DESCRIPTION = "Mainline Linux with custom patches"
AUTHOR = "joscha loos (joscha.loos@irs.systems)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

# which CVE product to target
CVE_PRODUCT = "linux_kernel"

# inherit kernel externalsrc
inherit kernel externalsrc

SRC_URI = "\
    file://kconfig.cfg \
    file://xilinx_defconfig \
"

COMPATIBLE_MACHINE = "adaq7768"

EXTERNALSRC = "/linux"
