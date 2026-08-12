SUMMARY = "Setup modprobe config files"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

S = "${WORKDIR}"
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://blacklist.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/modprobe.d/
    install -m 0644 ${WORKDIR}/blacklist.conf ${D}${sysconfdir}/modprobe.d/blacklist.conf
}
