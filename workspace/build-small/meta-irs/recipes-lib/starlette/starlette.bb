SUMMARY = "Python package for starlette"
HOMEPAGE = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://patch_pyproject.patch"

PV = "0.36.0"
PYPI_PACKAGE = "starlette"

SRC_URI[sha256sum] = "b1647c659133087906b83b762d5bb0d84ff3090c337de403c6e436354f53fc39"

DEPENDS += "python3-hatchling-native"

inherit pypi python_setuptools_build_meta
