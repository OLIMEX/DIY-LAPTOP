# Updating the image (Linux)

Once Linux boots, connect to the Internet either via on-board Wi-Fi adapter or by using an Ethernet cable.
You might use the graphical network manager or edit the configuration files via command line.
Then open a console terminal (press ALT+CTRL+T) and type:

```bash
sudo apt update
sudo apt upgrade
```

Insert your password if prompted and wait until each of the commands succeeds.

If you decide to completely re-write the image on the on-board [eMMC memory](glossary.md), follow the advice in the [next section](sw_fresh-os.md).
