# A64-OLinuXino Build Instructions

## Linux

### 1. Getting source code and helper scripts
	
```bash
mkdir a64-olinuxino
cd a64-olinuxino
git clone https://github.com/A64-TERES/linux-a64
git clone https://github.com/A64-TERES/u-boot_new
git clone https://github.com/hehopmajieh/arm-trusted-firmware-a64
git clone https://github.com/A64-TERES/blobs
git clone https://github.com/A64-TERES/sunxi-pack-tools sunxi-pack-tools -b pinebook
git clone https://github.com/A64-TERES/scripts
```
### 2. Setup toolchain
```bash
	sudo apt install gcc-aarch64-linux-gnu
        sudo apt install gcc-4.7-arm-linux-gnueabihf
	sudo apt install kpartx bsdtar mtools
```

### 3. Cross-compile sources

#### Linux
```bash
cd linux-a64
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olimex_teres1_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= clean
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j4 LOCALVERSION= Image
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j4 LOCALVERSION= modules
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j4 LOCALVERSION= modules_install  INSTALL_MOD_PATH=out INSTALL_MOD_STRIP=1
```
#### ATF
```bash
cd ../arm-trusted-firmware-a64/
make clean
make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu- PLAT=sun50iw1p1 bl31
```
#### Allwinner Pack Tools 
```bash
cd ../
make -C sunxi-pack-tools
```
#### U-Boot
```bash
cd scripts/
./build_uboot.sh #A64-Teres
```
or 
```bash
./build_uboot_a64.sh #A64-OLinuXino
```
### 4. Helper Scripts
## Ramdisk

Either make one with the steps below or download one from some other place.
Make sure the initrd is for aarch64.

### Get Busybox tree

```bash
cd ../
git clone --depth 1 --branch 1_24_stable --single-branch git://git.busybox.net/busybox busybox
```

### Configure and build Busybox

Build a static busybox for aarch64. Start by copying the `blobs/a64_config_busybox`
file to `.config` of your Busybox folder.

```bash
cp blobs/a64_config_busybox busybox/.config
cd busybox 
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j4 oldconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j4
```

### Make initrd.gz

Use the provided `make_initrd.sh` script to create a simple initrd based on
the busybox binary compiled earlier.

```bash
cd ../scripts
./make_initrd.sh
```
### Create bootable image

Create kernel tarball :
```bash 
./make_kernel_tarball.sh . ../linux-a64 #This will produce file named linux-a64-xx.yy.zz.tar.xz 
```

Create simple image structure :
```bash
sudo ./make_simpleimage.sh teres.img 1000 ./linux-a64-3.10.104-1.tar.xz 
sudo xz teres.img
```

Build bootable image :
```bash
sudo ./build_image.sh teres.img.xz ./linux-a64-3.10.104-1.tar.xz xenial
```

if everything is successfully acomplished this command will create file named :
xenial-teres-bspkernel-<date>.img
use dd to write this image to Sd Card : 
```bash
dd if=xenial-teres-bspkernel-<date>.img of=/dev/sdX bs=1M
```

After first boot you will able to login with : 
user: olimex
pass: olimex

Connection to internet can be enabled using nmtui tool:
```bash
nmtui
```

Feel free to install everything you want, for ex. Graphical desktop : 
```bash
./install_desktop.sh mate #will install mate 
```
