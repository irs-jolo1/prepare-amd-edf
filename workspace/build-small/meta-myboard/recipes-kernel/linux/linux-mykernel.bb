DESCRIPTION = "My custom Linux kernel"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit kernel

SRC_URI = "file://linux \
kconfig.cfg \
"
KCONFIG_FRAGMENTS += "kconfig.cfg"

SRCREV = "0123456789abcdef0123456789abcdef01234567"

S = "${WORKDIR}/linux"

COMPATIBLE_MACHINE = "zynqmp-generic-xczu2eg"

