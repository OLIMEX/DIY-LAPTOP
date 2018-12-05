# LCD(eDP)<->HDMI video switcher for Teres-I laptop

Script and DTB files that allow to switch video output medium 
from LCD(1366x768) to Dual display mode LCD(1366x768)+HDMI(1920x1080) and the other way around.

**WARNING! You need cable with mini HDMI connector to connect your monitor or TV 
(THE MINI HDMI CONNECTOR IS DIFFERENT FROM THE REGULAR HDMI CONNECTOR)**

	Ensure that you have a cable with proper connector and native HDMI monitor
	BEFORE YOU SET THE OUTPUT TO HDMI, ELSE IT WOULD BE HARD TO REVERT TO LCD.
 
If you set output to HDMI and you don't have HDMI monitor or 


How to install?

0. Download the files from this repository.

1. Copy files opt/teres... to /opt/teres recoursive. The command should be:
```
# sudo cp _path_to_file/teres /opt/teres -R
```
2. copy change-display in /usr/sbin directory
```
# sudo cp path_to_file/change-display /usr/local/sbin
```
3. Start the script change-display with 
```
# sudo change-display 
```
and follow the on-screen instructions.

4. Reboot the laptop.

## FAQ:

1. Can I have HDMI and LCD working at the same time?

A: Yes.

2. Does this work when I boot from the eMMC (e.g. I don't boot from a card)???

A: Yes.

