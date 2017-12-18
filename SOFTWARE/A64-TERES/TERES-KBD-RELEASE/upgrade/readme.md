How to update the touchscreen button firmware:

Download and extract the tgz archive. Open a console and navigate to the folder where you placed the archive then extract with the follwing command:

```bash
tar -xvf firmware.tar.gz
```
navigate to the firmware folder and then perform an update with the following commands: 
```bash
cd firmware 
sudo ./update    #standart firmware

or

sudo ./update-notap  #firmware with taps disabled
```
Then follow the onscreen prompts: 

Type the sudo password. 

Then press Fn+TuXkey+escape together to
start the update.

The firmware should now be updated.


Note: To get Fn+F1 suspend mode working with older releases:

1. Add
```bash
KEYBOARD_KEY_70071=suspend
```
at the end of file:
```bash
sudo nano /etc/udev/hwdb.d/teres_kbd.hwdb
```
2. You must create keyboard shortcut (System -> Preferences -> Hardware -> Keyboard Shortcuts -> Add) with name "Suspend" and command: "systemctl suspend" activated with key Fn+F1

3. reboot...
