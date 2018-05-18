# Updating the image

Once the Linux boots, connect to the Internet (either via the on-board WIFI adapter or using Ethernet cable).
You might use the graphical network manager or edit the configuration files via command line.
Then open a console terminal (press ALT+CTRL+T) and type:

```bash
sudo apt update
sudo apt upgrade
```

Insert your password if prompted and wait until each of the commands succeeds.

If you decide to completely re-write the image at the eMMC memory, follow the advice in the next chapter.
