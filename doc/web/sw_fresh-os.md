# Installing a fresh official image to TERES-I

In certain cases you might want to update the Linux on the eMMC memory (if the eMMC got corrupted; or if the original installation got damaged; or if there are no other means to update to the latest software release; etc).
The typical way of installing the operating system of TERES-I is via a micro SD card (there is a micro SD card slot at the right side of the TERES-I).
You would also need SD card writer.
Note that the whole procedure takes considerable amount of time and requires good internet connection.
The procedure is as follows:

1. Download and extract the latest image provided by Olimex.
A download link for the torrent with the latest image is available at the **SOFTWARE** section of the product's page.
It looks like this:

![Official OS Download](../images/TERES-I/software/screenshot-official-os.png "Official OS Download")

2. Remember to extract the image from the archive.
3. Download the img file to a micro SD card (8GB or bigger) using a micro SD card writer hardware.
If you don't know how, use free software like [Etcher](https://etcher.io/) - it has builds for Linux, Windows, and MacOS.
Download the installer for your operating system and start the software.
Select the image that you extracted.
Then point to the drive of the micro SD card.
It looks like this:

![Etcher Download](../images/TERES-I/software/screenshot-etcher.png "Etcher Download")

4. Insert the card in the laptop and start the laptop from the power button; it will boot from the micro SD card.
5. You will be asked for a username and password in a command line.
Username is ``olimex`` and the password is also ``olimex``.

6. Once you login execute the eMMC install script, that would move the files from the card to the built-in eMMC memory inside.
Type:

```bash
sudo install_emmc.sh
```

You will be asked for confirmation and then you would be prompted to select the emmc drive.
Select the defaults and wait until the install script completes (it might take up to 15 minutes).

7. Remove the card so TERES-I can now boot from the fresh image on the eMMC memory.

