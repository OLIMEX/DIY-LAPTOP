![OLIMEX Company Logo](doc/images/smflogo.png "OLIMEX Company Logo")
<img align="right" src="doc/images/TERES-I/TERES-A64-BLACK/laptop-12.jpg">

<br>

# TERES-I

<br><br>

## Overview

TERES-I is a Do-It-Yourself (DIY) Free/Open Source Hardware ([FOSH](https://wikipedia.org/wiki/Open-source_hardware)) and Software ([FOSS](https://wikipedia.org/wiki/Free_and_open-source_software)) laptop.
Stylish, elegant and lightweight, TERES-I is a convenient travel companion and is great for playing videos, browsing the web, or using a plethora of development suites.
It runs Linux on 64-bit ARM64 and x86 processors.

NOTE: TERES-I is currently considered an [evaluation board](doc/web/evaluation-board-notice.md) and not yet an end-product.

[Purchase your TERES-I here!](https://www.olimex.com/Products/DIY-Laptop/KITS)

| FEATURE              | DESCRIPTION                                                                                                         |
|:--------------------:|:-------------------------------------------------------------------------------------------------------------------:|
| **CPU**              | [Quad-Core Allwinner A64 64-bit Cortex-A53](doc/datasheets/Allwinner-A64/A64_Datasheet_V1.1.pdf)                    |
| **Memory**           | 2GB DDR3L                                                                                                           |
| **Internal Storage** | 16GB eMMC flash memory                                                                                              |
| **Monitor**          | [LCD 11.6" 1366x768 resolution](doc/datasheets/TERES-015-LCD11.6/N116BGE-EA2.pdf)                                   |
| **Video In**         | 1x [VGA 640x480 camera](HARDWARE/A64-TERES/TERES-019-Camera/N03A61B36DL32.pdf)                                      |
| **Video Out**        | 1x HDMI                                                                                                             |
| **Audio In/Out**     | 2x (stereo) speakers; 2x 3.5mm audio jack (1x microphone in, 1x stereo out)                                         |
| **Wireless**         | WiFi 150Mb; BLE 4.0                                                                                                 |
| **USB**              | 2x USB 2.0                                                                                                          |
| **Power**            | 1x 9500mAh [battery](doc/datasheets/LiPo-Battery/JA426992P2P-Spec-Data-Sheet-3.7V-7000mAh--161201.pdf); 5V/3A input |
| **Weight**           | 980 g (2.16 lb)                                                                                                     |

## [Hardware](HARDWARE)

* [TERES-A64-WHITE](https://www.olimex.com/Products/DIY-Laptop/KITS/TERES-A64-WHITE)
* [TERES-A64-BLACK](https://www.olimex.com/Products/DIY-Laptop/KITS/TERES-A64-BLACK)
* [Spare parts](https://www.olimex.com/Products/DIY-Laptop/SPARE-PARTS)
* [Assembling your TERES-I](doc/web/hw_assembly.md)

## [Software](SOFTWARE)

TERES-I was designed to run on Linux distributions, but may also run Android and Windows operating systems.
The laptop's mainboard has eMMC flash loaded with Ubuntu Mate and basic programs – Internet browser, Open Office, Arduino IDE, IceStorm FPGA Verilog tools, a video player, etc.
Additional software is also available from Ubuntu repositories.

* [Installing an image](doc/web/sw_fresh-os.md)
* [Updating an image](doc/web/sw_updating-os.md)
* Building an image
  * [Internal script](SOFTWARE/A64-TERES/scripts/README.md)
  * [Manual build reference](http://linux-sunxi.org/Manual_build_howto)

## Other resources

* [FAQ](doc/web/res_faq.md)
* [Troubleshooting](https://www.olimex.com/forum/index.php?board=39.0)
* [Assembly manuals (PDF)](doc/manuals)
* [What's next?](doc/web/res_next-steps.md)
* [Community Links](doc/web/res_community.md)
