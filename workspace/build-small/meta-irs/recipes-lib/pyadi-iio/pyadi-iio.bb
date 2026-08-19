SUMMARY = "Python package for pyadi-iio"
HOMEPAGE = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://pyproject.toml \
    file://adi \
"
# SRC_URI[sha256sum] = "0ebf0706d3e1a89886698509e71f42178bc58a5f03a47f7f9088f8d86900e7a7"

# PV = "0.0.20"
# PYPI_PACKAGE = "pyadi_iio"

S = "${WORKDIR}"

# inherit pypi python_setuptools_build_meta
inherit python_setuptools_build_meta

RDEPENDES:${PN} += "python3-numpy"
