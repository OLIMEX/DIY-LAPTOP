# LCD(eDP)<->HDMI video switcher for Teres-I laptop

Script and DTB files that allow to switch video output medium 
from LCD(1366x768) to HDMI(1280x720) and the other way around.

**WARNING! You need cable with mini HDMI connector to connect your monitor or TV 
(THE MINI HDMI CONNECTOR IS DIFFERENT FROM THE REGULAR HDMI CONNECTOR)**

	Ensure that you have a cable with proper connector and native HDMI monitor
	BEFORE YOU SET THE OUTPUT TO HDMI, ELSE IT WOULD BE HARD TO REVERT TO LCD.
 
If you set output to HDMI and you don't have HDMI monitor or 


How to install?

0. Download the files from this repository.

1. Copy a64-olinuxino-hdmi.dtb and a64-olinuxino-lcd.dtb in /boot/a64/ directory,
where the default a64-olinuxino.dtb file is located. The command should be:
```
# sudo cp _path_to_file/a64-olinuxino-hdmi.dtb /boot/a64/
# sudo cp _path_to_file/a64-olinuxino-lcd.dtb /boot/a64/
```
2. copy change-display in /usr/sbin directory
```
# sudo cp path_to_file/change-display /usr/sbin
```
3. Start the script change-display with 
```
# sudo change-display 
```
and follow the on-screen instructions.

4. Reboot the laptop.

## FAQ:

1. Can I have HDMI and LCD working at the same time?

A: It might be possible but we couldn't get it working. If you manage to do it,
please share what you did so we can update this guide, we would test and publish it.

2. I changed the video output to HDMI but I don't have a cable with fitting mini HDMI 
connector nor HDMI minotor? What now?

A: Either find a cable with fitting connector and HDMI monitor or re-write 
the official image to a micro SD card and transfer it to the eMMC. Refer to 
chapter 10 of the user's manual "10. Installing a fresh official image to TERES-I".

3. Does this work when I boot from the eMMC (e.g. I don't boot from a card)???

A: Yes.


First experimental release.
