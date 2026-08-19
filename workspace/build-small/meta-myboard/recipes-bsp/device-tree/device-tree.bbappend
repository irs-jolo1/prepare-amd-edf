FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# EXTRA_DT_INCLUDE_FILES:append:adaq7768 = " system-user.dtsi"

# the same as SRC_URI + automatic '#include' statement in device-tree
EXTRA_DT_INCLUDE_FILES:append = " system-user.dtsi"

# # Kernel dt-bindings headers (e.g. dt-bindings/gpio/gpio.h) are needed by
# # system-user.dtsi. The Linux (APU) build stages them under
# # ${STAGING_KERNEL_DIR}, but the microblaze-pmu / cortexa53-fsbl multiconfigs
# # don't. Point at the mounted kernel source tree so the header is available
# # to every multiconfig's DT preprocess step.
DT_INCLUDE:append = " /linux/include"
# DT_INCLUDE:append:adaq7768 = " ${STAGING_KERNEL_DIR}/include"


# &some_node {
#     /delete-property/ some-prop;
#     new-prop = <0x1>;
# };

# &amba {
#     /delete-node/ some_device;
# };
