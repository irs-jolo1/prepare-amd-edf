SUMMARY = "Install DNS services"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://resolv.conf \
    "

do_install() {
    install -d ${D}/etc
    install -m 644 ${WORKDIR}/resolv.conf ${D}/etc/resolv.conf
}

FILES:${PN} += "/etc/resolv.conf"
