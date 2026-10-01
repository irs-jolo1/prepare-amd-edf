MACHINE="adaq7768"

BOOTBIN=tmp/deploy/images/${MACHINE}/boot.bin

# bootgen -arch zynqmp -read $BOOTBIN
bootgen -arch zynqmp -verify $BOOTBIN


# echo "--> LIST"
# exbootimage -l $BOOTBIN

# echo "--> DESCRIPTION"
# exbootimage -d $BOOTBIN
