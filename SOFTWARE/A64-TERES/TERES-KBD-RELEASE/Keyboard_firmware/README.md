Factory default firmware. Burn it with AVR-ISP500(-TINY) using avrdude and command:
```bash
avrdude -p m32u4 -c stk500v2 -P /dev/ttyACM0 -U flash:w:keyboard.hex:a -U hfuse:w:0xD8:m -U efuse:w:0xC7:m -U lock:w:0xCF:m
```
or any ATmega32U4-compatible programmer tool and software.

An additional 2x3 male-male header is needed in order to connect the programmer
to the board.  It connects to the keyboard controller board, where the AVR chip
is, not the keyboard itself nor the mainboard. If the board is programmed
in-system, cut off the excess pin length that sticks out underneath the board
from the 2x3 header, so that soldered header pins do not press the board higher
than its intended mounting height, potentially damaging it if screwed in place.
Place a couple layers of tape under the pins to prevent the metal keyboard
housing from shorting them, which prevents the board from responding.

Note that the keyboard controller board may need to be externally powered in
order to write successfully,  It seems sufficient to plug it into a fully
powered running mainboard.  When working under the system with the battery and
touchpad below the keyboard controller board, the cable for the AVR-ISP500
should be heading off to your left from the 2x3 header it plugs in to, if
oriented correctly.

The keyboard control board has pin labels on the underside.  From above, when
working with the system upside down and the keyboard below the viewer, they
would be:

	                                  GND 6    o o    5 RST
	==== cable to programmer ====    MOSI 4    o o    3 SCK
	                                  VDD 2    o o    1 MISO

The pinout from the AVR-ISP500, looking into the female 2x3 plug with the cable heading to the right, is:

	 RST 5    o o    6 GND
	 SCK 3    o o    4 MOSI   ==== cable to programmer ====
	MISO 1    o o    2 VDD

Explanation of avrdude options, from the avrdude man page:

	`-V`: skip verify step
	`-p m32u4`: specifies the chip on the board
	`-c stk500v2`: specifies the programmer used
	`-P /dev/ttyACM0`: specifies the serial device to find the programmer
	`-U flash:w:keyboard.hex:a`: [w]rites keyboard.hex to the flash ROM of the device, [a]utodetecting format
	`-U hfuse:w:0xD8:m`: [w]rites the im[m]ediate value 0xD8 to the high fuse byte
	`-U efuse:w:0xC7:m`: [w]rites the im[m]ediate value 0xC7 to the extended fuse byte
	`-U lock:w:0xCF:m`: [w]rites the im[m]ediate value 0xCF to the lock byte

	Other useful -U options: [r]ead to retrieve data from the chip, [r]aw format for binary, [i]ntel format for hex

The list of the avrdude m32u4 memory types can be found from the avrdude `part` command in terminal (`-t`) mode:

```
AVR Part                      : ATmega32U4
Chip Erase delay              : 9000 us
PAGEL                         : PD7
BS2                           : PA0
RESET disposition             : dedicated
RETRY pulse                   : SCK
serial program mode           : yes
parallel program mode         : yes
Timeout                       : 200
StabDelay                     : 100
CmdexeDelay                   : 25
SyncLoops                     : 32
ByteDelay                     : 0
PollIndex                     : 3
PollValue                     : 0x53
Memory Detail                 :

                         Block Poll               Page                       Polled
  Memory Type Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack
  ----------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------
  eeprom        65    20     4    0 no       1024    4      0  9000  9000 0x00 0x00
  flash         65     6   128    0 yes     32768  128    256  4500  4500 0x00 0x00
  lfuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
  hfuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
  efuse          0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
  lock           0     0     0    0 no          1    0      0  9000  9000 0x00 0x00
  calibration    0     0     0    0 no          1    0      0     0     0 0x00 0x00
  signature      0     0     0    0 no          3    0      0     0     0 0x00 0x00
```
