FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://autologin \
"


do_install() {
	install -d ${D}${sysconfdir}
	install -D -m 0644 ${WORKDIR}/inittab ${D}${sysconfdir}/inittab
	install -D -m 0744 ${WORKDIR}/autologin ${D}${sysconfdir}/autologin


    cat >> ${D}${sysconfdir}/inittab <<EOF



ttyPS0::respawn:/sbin/getty -n -l ${sysconfdir}/autologin -L 115200 ttyPS0
EOF

}

FILES:${PN} += "${sysconfdir}/autologin"
