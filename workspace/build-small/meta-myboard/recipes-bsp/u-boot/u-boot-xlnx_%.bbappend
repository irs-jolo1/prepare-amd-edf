FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " file://myboard.cfg \
"

# Point at your defconfig
# UBOOT_MACHINE = "myboard_defconfig"

# Merge extra config fragment (requires kconfig class support)
do_configure:append() {
    cat ${WORKDIR}/myboard.cfg >> ${B}/.config
}

# > bitbake -e irs-image | grep "^UBOOT_"
# UBOOT_BINARY="u-boot.bin"
# UBOOT_BOOT_SCRIPT="u-boot-edf-scr"
# UBOOT_BOOT_SCRIPT:amd-edf="u-boot-edf-scr"
# UBOOT_ELF="u-boot.elf"
# UBOOT_ENTRYPOINT="0x200000"
# UBOOT_LOADADDRESS="0x200000"
# UBOOT_MACHINE="xilinx_zynqmp_virt_defconfig"
# UBOOT_SUFFIX="bin"
