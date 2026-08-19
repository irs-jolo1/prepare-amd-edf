SUMMARY = "Python package for fastapi"
HOMEPAGE = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# SRC_URI += "file://patch_license.patch"

# PV = "0.135.3"
PV = "0.116.1"
PYPI_PACKAGE = "fastapi"

SRC_URI[sha256sum] = "ed52cbf946abfd70c5a0dccb24673f0670deeb517a88b3544d03c2a6bf283143"


DEPENDS += "python3-pdm"
DEPENDS += "python3-pdm-backend-native"
RDEPENDS:${PN} += "starlette"
RDEPENDS:${PN} += "python3-pydantic"
# RDEPENDS:${PN} += "python3-pydantic (>=2.7)"
RDEPENDS:${PN} += "python3-annotated-doc"
RDEPENDS:${PN} += "python3-pydantic-core"
RDEPENDS:${PN} += "python3-json"
RDEPENDS:${PN} += "python3-anyio"
RDEPENDS:${PN} += "python3-typing-extensions"
RDEPENDS:${PN} += "python3-annotated-types"

inherit pypi python_setuptools_build_meta
