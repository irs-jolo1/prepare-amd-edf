SUMMARY = "Python package for uvicorn"
HOMEPAGE = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://patch_pyproject.patch"

PV = "0.35.0"
PYPI_PACKAGE = "uvicorn"

SRC_URI[sha256sum] = "bc662f087f7cf2ce11a1d7fd70b90c9f98ef2e2831556dd078d131b96cc94a01"

DEPENDS += "python3-hatchling-native"
RDEPENDS:${PN} += "python3-h11"
RDEPENDS:${PN} += "python3-click"
RDEPENDS:${PN} += "python3-typing-extensions"
RDEPENDS:${PN} += "python3-websockets"

inherit pypi python_setuptools_build_meta
