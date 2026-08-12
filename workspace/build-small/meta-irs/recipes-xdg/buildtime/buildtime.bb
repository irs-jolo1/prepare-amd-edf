SUMMARY = "Place file into root directory with build information"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

S = "${WORKDIR}"

FILES:${PN} += "/root/linux_build_info.json /root/fpga_build_info.json"

DEPENDS += "unzip-native jq-native"

XSA_FILE = "${TOPDIR}/../project-spec/hw-description/system.xsa"

do_compile[nostamp] = "1"
do_install[nostamp] = "1"

do_install:append() {

    install -d ${D}/root/

    ${STAGING_BINDIR_NATIVE}/unzip -p ${XSA_FILE} buildinfo.json | ${STAGING_BINDIR_NATIVE}/jq '.' > ${D}/root/fpga_build_info.json

    BUILD_TIME="$(TZ=Europe/Berlin date --rfc-email)"

    ${STAGING_BINDIR_NATIVE}/jq -n \
        --arg build_time "${BUILD_TIME}" \
        --arg ci_build "${CI}" \
        '{build_time: $build_time, ci_build: $ci_build}' \
        > ${D}/root/linux_build_info.json
}
