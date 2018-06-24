# Touchpad & button firmware update

1. Download and extract the tgz archive:

```bash
tar -xvf firmware.tar.gz
```

2. Navigate into the newly created folder where the archive was extracted:

```bash
cd firmware/
```

3. Perform an update:

```bash
sudo ./update #standard firmware
```

or

```bash
sudo ./update-notap #firmware with taps disabled
```

4. Follow the onscreen prompts: 

- Enter the superuser password
- Press Fn+TuXkey+escape simultaneously to begin the update

The firmware should now be updated!

## To get Fn+F1 suspend mode working with older releases

1. Run the following command:

```bash
echo "KEYBOARD_KEY_70071=suspend" | sudo tee -a /etc/udev/hwdb.d/teres_kbd.hwdb
```

2. Map a keyboard shortcut (System -> Preferences -> Hardware -> Keyboard Shortcuts -> Add)

- Name: `Suspend`
- Command: `systemctl suspend`
- Key binding: `Fn+F1`

3. Reboot:

```bash
sudo shutdown -r now
```
