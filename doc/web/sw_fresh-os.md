# Installing a fresh image on your TERES-I

## Linux

You may sometimes wish to update Linux on the [eMMC memory](glossary.md).
This may be if the eMMC got corrupted, the original installation got damaged, or if there are no other means to update to the latest software releases, etc).
The standard method for installing an operating system onto the TERES-I is via a [micro SD](glossary.md) card.
(There is a micro SD card slot at the right side of the TERES-I).
You would also need SD card writer.
Note that the entire procedure takes a considerable amount of time and requires stable internet connection.
The procedure is as follows:

1. Download and extract the latest image provided by Olimex.
A download link for the torrent with the latest image is available at the **SOFTWARE** section of the product's page.
It looks like this:

![Official OS Download](../images/TERES-I/software/screenshot-official-os.png "Official OS Download")

2. Remember to extract the image from the archive.
3. "Burn" the img file to a micro SD card (8GB or larger) using a micro SD card writer method of your choice.
The ``dd`` command works great for this task.
For the following example, everything between ``<>`` needs to be modified to fit your situation:

```bash
lsblk
```

Note the ``/dev`` path to your eMMC device.

```bash
sudo dd if=</path/to/image-to-load.img> of=/dev/<eMMC-device-name> bs=1M status=progress
```

If you are not comfortable with this procedure, graphical tools are available such as [Etcher](https://etcher.io).
Etcher is compatible with Linux, Windows, and MacOS, and you may download the installer for your host operating system and start the software.

To use Etcher:
Select the image that you extracted.
Then point to the drive of the micro SD card.
It looks like this:

![Etcher Download](../images/TERES-I/software/screenshot-etcher.png "Etcher Download")

4. Insert the micro SD card into your TERES-I and power on the laptop using the power button; it will boot from the micro SD card.
5. You will be asked for a username and password in a command line.
Username=``olimex``, password=``olimex``

6. Once you log in, execute the eMMC installer script.
This script copies the contents from the microSD card to the built-in eMMC memory inside your TERES-I.
To do this, type:

```bash
sudo install_emmc.sh
```

You will be asked for confirmation and then you would be prompted to select the emmc drive.
Select the defaults and wait until the install script completes.
This script may take up to 15 minutes to complete.

7. Remove the micro SD card in order for your TERES-I to boot from the freshly-loaded image in its eMMC memory.

#### Congratulations - task complete!
