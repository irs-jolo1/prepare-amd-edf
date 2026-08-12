#
# This file is the irsutils recipe.
#

SUMMARY = "Simple irsutils application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://pyproject.toml \
    file://src \
    file://irsutils_healthcheck.service \
    "

S = "${WORKDIR}"

FILES:${PN} += "${systemd_system_unitdir}/irsutils_healthcheck.service"

inherit python_setuptools_build_meta

RDEPENDS:${PN} += "python3"
RDEPENDS:${PN} += "python3-core"
RDEPENDS:${PN} += "pyadi-iio"
RDEPENDS:${PN} += "python3-numpy"
RDEPENDS:${PN} += "fastapi (=0.116.1)"
RDEPENDS:${PN} += "uvicorn"
RDEPENDS:${PN} += "python3-requests"
RDEPENDS:${PN} += "python3-websocket-client"
RDEPENDS:${PN} += "python3-packaging"

inherit systemd

SYSTEMD_SERVICE:${PN} = "${@'irsutils_healthcheck.service' if d.getVar('CI') == 'true' else ''}"
SYSTEMD_AUTO_ENABLE:${PN} = "${@'enable' if d.getVar('CI') == 'true' else 'disable'}"

do_install:append() {
    if [ "${CI}" = "true" ]; then
        sed -i 's/<token>/${WEBHOOK_TOKEN}/' ${WORKDIR}/irsutils_healthcheck.service
        install -d ${D}${systemd_system_unitdir}
        install -m 0644 ${WORKDIR}/irsutils_healthcheck.service \
            ${D}${systemd_system_unitdir}/
    fi
}
