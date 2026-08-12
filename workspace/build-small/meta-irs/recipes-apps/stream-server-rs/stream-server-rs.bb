SUMMARY = "Recipe for the rust-based stream server"
HOMEPAGE = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://stream-server-rs \
    file://stream-server-rs.service \
    "
FILES:${PN} += "${systemd_system_unitdir}/stream-server-rs.service"

S = "${WORKDIR}/stream-server-rs"


TARGET_BIN_NAME = "stream-server-rs"

inherit cargo

# set correct build dir
# B = "${S}"
CARGO_SRC_DIR = ""

do_fetch[network] = "1"
do_compile[network] = "1"

CARGO_BUILD_FLAGS:remove = "--frozen"
CARGO_DISABLE_BITBAKE_VENDORING = "1"


DEPENDS += "rust-native cargo-native"
DEPENDS += "libiio"

# generate with `cargo vendor` in rust project root
# CARGO_VENDORING_DIRECTORY := "${S}/vendor"


inherit systemd

SYSTEMD_SERVICE:${PN} = "stream-server-rs.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
  install -d ${D}/${systemd_system_unitdir}/
  install -m 0644 ${WORKDIR}/stream-server-rs.service ${D}/${systemd_system_unitdir}/
}
