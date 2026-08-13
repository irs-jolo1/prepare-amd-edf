FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://autologin \
"


# sed -i \
#     's#^tty1:12345:respawn:/usr/sbin/getty 38400 tty1$#tty1:12345:respawn:/usr/sbin/getty -n -l /bin/autologin 38400 tty1#' \
#     ${D}${sysconfdir}/inittab
do_install() {
	install -d ${D}${sysconfdir}
	install -D -m 0644 ${WORKDIR}/inittab ${D}${sysconfdir}/inittab
	install -D -m 0744 ${WORKDIR}/autologin ${D}${sysconfdir}/autologin

    cat >> ${D}${sysconfdir}/inittab <<EOF
ttyPS0::respawn:/sbin/getty -n -l ${sysconfdir}/autologin -L 115200 ttyPS0
EOF

}
# echo "tty1:12345:respawn:/usr/sbin/getty -L tty1 115200" >> ${D}${sysconfdir}/inittab
# sed -i \
#     's#^tty1:12345:respawn:/usr/sbin/getty 38400 tty1$##' \
#     ${D}${sysconfdir}/inittab

# echo "tty1:12345:respawn:/bin/login -f root" >> ${D}${sysconfdir}/inittab

FILES:${PN} += "${sysconfdir}/autologin"
