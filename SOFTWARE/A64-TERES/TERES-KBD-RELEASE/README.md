# TERES-KBD-RELEASE

The purpose of this document is to describe how to compile the keyboard and touchpad firmware for the TERES-I.
Make sure that you are performing these steps on the TERES-I itself.

1. Go to this directory
```bash
cd SOFTWARE/A64-TERES/TERES-KBD-RELEASE/
```
2. Download Dean Camera's LUFA USB stack [here](http://www.fourwalledcubicle.com/LUFA.php).
Extract the archive inside [TERES-KBD-RELEASE/](.) to directory lufa-LUFA-170418
```bash
unzip lufa-LUFA-170418.zip
```
3. The Olimex keyboard + touchpad code is located in [TERES-KBD-RELEASE/TERES-HID/](TERES-HID).
Navigate there to edit the build dependencies:
```bash
cd TERES-HID/
```
4. Edit the makefile inside [TERES-KBD-RELEASE/TERES-HID/](TERES-HID):
```bash
nano makefile
```
search for `LUFA_PATH` and make sure it point to the LUFA library:
```bash
LUFA_PATH    = ../lufa-LUFA-170418/LUFA
```
Save the file and exit the text editor.

5. Compile:
```bash
make
```
6. Update the firmware of the TERES-I's keyboard and touchpad firmware:
```bash
sudo ./update
```
You will be prompted to press "fn+Tux+ESC" (function + penguin + escape) keys simultanously at some point.
Make sure to do so!

7. Finished!

**Note 1**: If you wish to play with the behavior yourself edit the sources in [TERES-KBD-RELEASE/TERES-HID/](TERES-HID).

**Note 2**: If you have problems with the touchpad and it seems dead, make sure that you haven't disabled it with fn+F3.
