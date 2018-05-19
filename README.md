![OLIMEX Company Logo](doc/images/smflogo.png "OLIMEX Company Logo")
<img align="right" src="doc/images/TERES-I/TERES-A64-BLACK/laptop-12.jpg">

<br>

# TERES-I Laptop

<br><br>

## Overview

TERES-I is a Do-It-Yourself (DIY) Free/Open Source Hardware (FOSH) and Software (FOSS) laptop design leveraging ARM64 and x86 processors.
DIY kits are ready-to-assemble using the instructions provided and may be found [here](https://www.olimex.com/Products/DIY-Laptop/KITS/).

* [What is TERES-I?](doc/web/intro_what-is-teres-i.md)
* [Where does the name come from?](doc/web/intro_name-origin.md)
* [What makes this laptop different?](doc/web/intro_what-is-unique.md)
* [Why does open-source matter so much?](doc/web/intro_importance-of-open-source.md)
* [Where are the sources?](doc/web/intro_sources.md)

## Hardware

Open-source hardware delivers transparency and freedom to the user (you) as you are able to tailor the PCBs according to your needs.
You are able to add more processors, memories, etc.
All of the hardware schematics and board files are designed using [KiCad](http://kicad-pcb.org) FOSS software.

> KiCad is free software. KiCad is made available under the GNU General Public License(GPL) version 3 or greater.

All KiCad design files are available in this repository.

Different main board configurations: the first iterration comes with ARM64 and x86 and later MIPS architecture.
Other architectures may follow.
The concept is to make templates which others may use to customize so other SOCs can be used for a main board.
TERES-I comes with 1376x768 LCD configuration. Future main boards aim at 1920x1080 pixels.

TERES-I is environmentally friendly.
The main board may be upgraded without major revisions to the design, vastly reducing waste and time when new faster and better processors become available.
Spare parts are also available for purchase.

* [Introduction to the hardware of TERES-I](doc/web/hw_intro.md)
* [What do you get in the package?](doc/web/hw_in-the-box.md)

## Software

TERES-I primarily runs on Linux distributions, but Android and Windows are also possible.

* [Installing a fresh official image to TERES-I](doc/web/sw_fresh-os.md)
* [Updating the image](doc/web/sw_updating-os.md)
* [Building the software](doc/web/sw_building.md)

## Laptop Assembly

Assembling this DIY laptop you would probably learn a lot of new things.
You can build it together with your kids or students, sparking interest in electronics and technology in general.
You will know every bit of your laptop and will be able to fix if something doesn't suit your needs or gets damaged.

* [How do I assemble my TERES-I?](doc/web/hw_assembly.md)

## Resources

* [TERES-A64-WHITE](https://www.olimex.com/Products/DIY-Laptop/KITS/TERES-A64-WHITE/open-source-hardware)
* [TERES-A64-BLACK](https://www.olimex.com/Products/DIY-Laptop/KITS/TERES-A64-BLACK/open-source-hardware)
* [Spare parts](https://www.olimex.com/Products/DIY-Laptop/SPARE-PARTS/)

* [FAQ](doc/web/res_faq.md)
* [Troubleshooting](doc/web/res_troubleshooting.md)
* [Download manuals](doc/manuals)
* [What's next?](doc/web/res_next-steps.md)
* [Community Links](doc/web/res_community.md)
