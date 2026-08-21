SUMMARY = "IRS development image"
DESCRIPTION = "Based on irs-image, with some additional programs useful for development"
AUTHOR = "Joscha Loos (joscha.loos@irs.systems)"
LICENSE = "MIT"
CVE_PRODUCT = ""

require recipes-core/images/irs-image.bb

IMAGE_INSTALL:append = "\
    dtc \
    htop \
    opkg \
    libiio-tests \
"
