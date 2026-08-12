SUMMARY = "Setup user home dir on device"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://bash_history.sh \
   file://bashrc.sh \
    "
FILES:${PN} += "/root/.bash_history \
    /root/.bashrc \
    "

S = "${WORKDIR}"

do_install:append() {
  install -d ${D}/root/
  install -m 0644 ${WORKDIR}/bash_history.sh ${D}/root/.bash_history
  install -m 0644 ${WORKDIR}/bashrc.sh ${D}/root/.bashrc
}
