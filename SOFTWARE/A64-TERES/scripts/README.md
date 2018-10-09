# Build Instructions

## Linux

### 1. Getting source code and helper scripts
```bash
cd ~/
git clone https://github.com/OLIMEX/DIY-LAPTOP
cd DIY-LAPTOP/SOFTWARE/A64-TERES/
```
### 2. Setup toolchain
```bash
sudo apt install gcc-aarch64-linux-gnu
sudo apt install gcc-4.7-arm-linux-gnueabihf
sudo apt install kpartx bsdtar mtools dos2unix device-tree-compiler
```

### 3. Cross-compile sources

#### Linux
```bash
cd linux-a64
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olimex_teres1_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- LOCALVERSION= clean
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j`nproc` LOCALVERSION= Image
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j`nproc` LOCALVERSION= modules
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j`nproc` LOCALVERSION= modules_install  INSTALL_MOD_PATH=out INSTALL_MOD_STRIP=1
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
Make sure the initrd is for `aarch64`.

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
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j`nproc` oldconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j`nproc`
```

### Make initrd.gz
Use the `make_initrd.sh` script to create a simple initrd based on the busybox binary previously compiled.

```bash
cd ../scripts
./make_initrd.sh
```
### Create bootable image
Create kernel tarball :
```bash 
./make_kernel_tarball.sh . ../linux-a64 #This will produce file named linux-a64-xx.yy.zz.tar.xz 
```

Create simple image structure:
```bash
sudo ./make_simpleimage.sh teres.img 1000 ./linux-a64-3.10.104-1.tar.xz 
sudo xz teres.img
```

Build a bootable image:
```bash
sudo ./build_image.sh teres.img.xz ./linux-a64-3.10.104-1.tar.xz xenial
```

If successfully completed, this command will create file named `xenial-teres-bspkernel-<date>.img`.
Use `dd` to write this image to an SD Card:

```bash
dd if=xenial-teres-bspkernel-<date>.img of=/dev/sdX bs=1M status=progress
```

Use these credentials to log in after first boot:
- User: `olimex`
- Password: `olimex`

Connection to the internet can be enabled via command-line by running:
```bash
nmtui
```

Feel free to install everything you want, e.g., a graphical desktop environment: 
```bash
./platform-scripts/install_desktop.sh mate # installs mate desktop
```
