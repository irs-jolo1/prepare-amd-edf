SUMMARY = "Enable DHCP"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

REQUIRED_DISTRO_FEATURES = "systemd"

SRC_URI = " \
    file://10-end0.network \
    "

# RDEPENDS:${PN} = "wpa-supplicant"

# do_configure[noexec] = "1"
# do_compile[noexec] = "1"

do_install() {
    install -d ${D}/etc/systemd/network
    install -m 644 ${WORKDIR}/10-end0.network ${D}/etc/systemd/network/10-end0.network
}

# pkg_postinst_ontarget:${PN}() {
#     if systemctl > /dev/null 2>&1 ; then
#         systemctl enable wpa_supplicant@wlan0.service
#     fi
# }

FILES:${PN} += "/etc/systemd/network"
