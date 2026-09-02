MACHINE="adaq7768"

echo "Removing old files"
rm /media/joscha/ZYNQMP/*

echo "Copying new files"
cp -L tmp/deploy/images/${MACHINE}/boot.bin /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/Image.lzma /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/system.dtb /media/joscha/ZYNQMP
cp -L tmp/deploy/images/${MACHINE}/irs-image*adaq7768.rootfs.cpio.gz.u-boot /media/joscha/ZYNQMP/cpio.gz.u-boot
cp -L tmp/deploy/images/${MACHINE}/fitImage /media/joscha/ZYNQMP/fitImage.bin
cp -L tmp/deploy/images/${MACHINE}/boot.scr /media/joscha/ZYNQMP

echo "Syncing"
sync
