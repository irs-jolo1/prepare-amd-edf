SUMMARY = "Combined FIT image: kernel + initramfs + FPGA bitstream"
LICENSE = "MIT"

DEPENDS = "u-boot-mkimage-native dtc-native virtual/kernel"

# Make sure the pieces we incbin exist in DEPLOY_DIR_IMAGE before we run
do_compile[depends] += " \
    virtual/kernel:do_deploy \
    ${INITRAMFS_IMAGE}:do_image_complete \
    fpga-bitstream:do_deploy \
"

#INITRAMFS_IMAGE ?= "core-image-minimal-initramfs"
INITRAMFS_IMAGE ?= "irs-image"
FPGA_BITSTREAM_FILE ?= "system.bit.bin"
KERNEL_DEVICETREE_NAME ?= "${MACHINE}.dtb"

SRC_URI = "file://fitImage.its"

S = "${WORKDIR}"
B = "${WORKDIR}/build"

do_compile() {
    mkdir -p ${B}
    cp ${S}/fitImage.its ${B}/
    cp ${DEPLOY_DIR_IMAGE}/Image ${B}/Image
    cp ${DEPLOY_DIR_IMAGE}/${KERNEL_DEVICETREE_NAME} ${B}/system.dtb
    cp ${DEPLOY_DIR_IMAGE}/${INITRAMFS_IMAGE}-${MACHINE}.rootfs.cpio ${B}/rootfs.cpio
    cp ${DEPLOY_DIR_IMAGE}/${FPGA_BITSTREAM_FILE} ${B}/system.bit.bin

    cd ${B}
    uboot-mkimage -f fitImage.its fitImage
}

do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 ${B}/fitImage ${DEPLOYDIR}/fitImage-combined-${MACHINE}.itb
}
addtask deploy after do_compile
