FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " file://myboard.cfg \
"

# Point at your defconfig
# UBOOT_MACHINE = "myboard_defconfig"

# Merge extra config fragment (requires kconfig class support)
do_configure:append() {
    cat ${WORKDIR}/myboard.cfg >> ${B}/.config
}
