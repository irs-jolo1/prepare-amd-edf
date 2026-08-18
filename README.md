# AMD EDF based IRS Linux Distro

- [link](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/2824503297/Building+Linux+Images+Using+Yocto)
- [yocto-xlnx](https://github.com/Xilinx/yocto-manifests.git)
- [petalinux vs edf vs yocto](https://bltinc.com/2026/07/08/petalinux-vs-edf-vs-yocto/)

## Getting Started

- Clone this repo
- Make sure your user is in the `docker group`
- run `docker build -t amd-edf . && docker compose run --rm amd-edf bash`
- You are now in the docker container. Everything EDF related is supposed to run in here, unless specified otherwise

## File overview

```
workspace/
├── conf/
│   ├── machine # generated machine config
│   └── local.conf # This is the file all docs are refering to
└── meta-myboard/
    ├── recipes-bsp/
    │   ├── device-tree
    │   └── u-boot
    ├── recipes-core/
    │   ├── busybox
    │   └── images
    └── recipes-kernel/
        ├── linux
        └── (fitimage)
```


## Generate SDT and yocto machine config from .xsa

- `stdgen` is istalled alongside Vivado/Vitis
- Run `stdgen` to generate the SDT
```
sdtgen -xsa system.xsa -dir workspace/sdt/sdt
```

(It's important that the SDT is in `workspace/sdt/sdt`)

- Next, parse the SDT to generate the machine config
- `gen-machineconf` (old) and `gen-machine-conf` (new) are currently equivalent

```
gen-machine-conf parse-sdt \
    --hw-description <System Device-tree Directory> \
    --machine-name <Name for your machine/board>
```

## Include Packages

Edit the `IMAGE_INSTALL` variable in `irs-image.bb`

```
IMAGE_INSTALL= "\
 base-files \
 base-passwd \
 busybox \
 initscripts \
 libiio-tests \
 ${CORE_IMAGE_EXTRA_INSTALL} \
"
```


## Add Kernel config

- Edit Kernel config (like activating a kernel module), add it to `kconfig.cfg`
- The value can either be `m` (module), `y` active, or `n` (disabled)

```
CONFIG_AD7768_1=m
```

## Build the image

- `bitbake irs-image`
- The image is defined in `meta-myboard/recipes-core/images/irs-image.bb`


## Other commands

```
docker build -t yocto-xlnx . && docker compose run --rm yocto-xlnx bash
source setupsdk build-small
bitbake core-image-custom-tiny
bitbake -e | grep "^DISTRO_FEATURES="
bitbake -e <image-name> | grep "^IMAGE_FEATURES="
bitbake -e | grep "^DISTRO_FEATURES:append"
bitbake -e | grep "^DISTRO_FEATURES:remove"
bitbake-layers show-recipes cmake
MACHINE=zcu102-zynqmp bitbake petalinux-image-minimal
```

```
bitbake-layers create-layer meta-myboard
bitbake-layers add-layer meta-myboard
bitbake-layers show-layers
bitbake virtual/kernel -c menuconfig
yocto@yocto-xlnx:/workspace/build-small$ bitbake -e virtual/kernel | grep '^PREFERRED_PROVIDER_virtual/kernel'

bitbake-layers show-appends | grep busybox-inittab
bitbake-layers show-recipes busybox-inittab

bitbake -u taskexp
source toaster start webport=0.0.0.0:8000

oe-pkgdata-util find-path /etc/inittab
oe-pkgdata-util list-pkg-files libiio-utils

bitbake -e libiio | grep ^PACKAGES= # find which packages are available by libiio

gen-machine-conf --hw-description </path/to/xsa-file> --machine-name adaq7768 --output .. # then use generated MACHINE

# recursively get where the variable MACHINE was set or changed
# examples:
bitbake-getvar -r virtual/kernel MACHINE
bitbake-getvar MACHINE
bitbake-getvar PV
bitbake-getvar PREFERRED_PROVIDER_virtual

# todo
load mmc 0:1 ${kernel_addr_r} Image-initramfs-<machine>.bin
booti ${kernel_addr_r} - ${fdt_addr_r}
```

with `bitbake -c menuconfig virtual/kernel`
- enable tty and related for xlnx, otherwise kernel does not output to tty/console
- enable BINFMT_ELF and related [from here](https://unix.stackexchange.com/questions/708779/linux-kernel-boot-failed-to-execute-init-error-2)

also:

- fix systemd cannot load tmpfs and panics
- but into `meta-kernel/linux/kconfig.cfg`
```
CONFIG_DEVTMPFS=y
CONFIG_DEVTMPFS_MOUNT=y
```

- inspect cpio fs
`gzip -dc irs-image-adaq7768.rootfs.cpio.gz | cpio -idmv`

## Deprecated XSCT

- I think what they mean is to use
```
gen-machineconf --hw-description ../sdt/sdt --machine-name adaq7768 # then use generated MACHINE
```

- However, I am confused because the current version of this script is still based on xsct, so the error still appears, but the output is a system device


## U-Boot

- Boots from fat partition (on SDCARD) from file `BOOT.BIN` (has to be named like this)

### Commands


```
version # shows version information
printenv # print all env information
bdinfo ## show important values and addresses
mmc # see below
printenv kernel_addr_r fdt_addr_r ramdisk_addr_r loadaddr # print where each image should be stored in memory
md.b ${kernel_addr_r} 80 # show the raw bytes in hex format
```


### Important env variables

| variable | expected value |
|----------|----------------|
| baudrate | 115200 |
| board | zynqmp |
| board_name | zynqmp |
| bootcmd | |
| boot delay | |


### Inspect MMC storage

```
mmc list # show storage devices
mmc dev 1 # select storage device 1
mmc info # show selected storage info
mmc part # show storage partitions

fatls mmc 1:1 # equivalent to `ls` on linux (for FAT)
ext4 mmc 1:1 # like fatls but for EXT4

iminfo [<addr>] # inspect image
```

### Boot Linux image

```
fatload mmc 1:1 0x80000000 Image
fatload mmc 1:1 0x83000000 gpio.gz.u-boot

bootm # boot legacy fit image
booti # boot linux `Image`
bootz # boot compressed `zImage`
```

### Device Tree

```
fatload mmc 1:1 ${fdt_addr_r} system.dtb
fdt addr ${fdt_addr_r}
fdt print [<node>]
fdt print /chosen # print bootargs
```


### Current Boot Sequence


```
setenv bootargs "earlycon console=ttyPS0,115200 systemd.log_target=console"

fatload mmc 1:1 ${kernel_addr_r} Image
fatload mmc 1:1 ${fdt_addr_r} system.dtb
fatload mmc 1:1 ${ramdisk_addr_r} cpio.gz.u-boot

booti ${kernel_addr_r} ${ramdisk_addr_r}:${ramdisk_size} ${fdt_addr_r}
```


### INITRAMFS

- [docs](https://docs.amd.com/r/en-US/ug1144-petalinux-tools-reference-guide/Configuring-INITRAMFS-Boot?tocId=9UMmYIMZythpvg8RQ6YbCA)


## EDF

- [link](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/3258089693/Operating+System+Integration+and+Development+AMD+Versal+device+portfolio#Yocto-Project-build-setup-instructions-for-EDF)
- [how it all works](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/3250586081/Development+Flows+-+How+it+all+works#Creating-a-image-for-multi-stage-boot-(Primary-Boot-Device-%E2%86%92-Secondary-Boot-device))
