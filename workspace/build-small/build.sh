MACHINE="adaq7768"

cp -L tmp/deploy/images/${MACHINE}/boot.bin /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/Image.lzma /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/system.dtb /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/irs-image-adaq7768.rootfs.cpio.gz.u-boot /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/boot.scr /media/joscha/ZYNQMP

sync
