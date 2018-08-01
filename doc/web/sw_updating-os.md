# Updating the image (Linux)

Important! Currently the image upgrade is possible only via a clean install (via SD card). Running "sudo apt upgrade" 
would lead to unexpected results and problems! Avoid running "apt upgrade"!

If you decide to completely re-write the image on the on-board [eMMC memory](glossary.md), follow the advice in the [next section](sw_fresh-os.md).

In order to just update it is safe to run "sudo apt update" Once Linux boots, connect to the Internet either
via on-board Wi-Fi adapter or by using an Ethernet cable.You might use the graphical network manager or edit 
the configuration files via command line.

Then open a console terminal (press ALT+CTRL+T) and type:

```bash
sudo apt update
```

Insert your password if prompted and wait until the commands succeeds.
