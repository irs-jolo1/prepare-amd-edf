

# load our files dir first
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# add boot cmd file
SRC_URI += "file://edf-linux-boot.cmd"
