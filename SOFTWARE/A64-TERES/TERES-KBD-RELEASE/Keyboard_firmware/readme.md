Factory default firmware. Burn it with AVR-ISP500(-TINY) using avrdude and command:
```bash
avrdude -V -pm32u4 -cstk500v2 -P/dev/ttyACM0 -U flash:w:keyboard.hex:a -U hfuse:w:0xD8:m -U efuse:w:0xC7:m -U lock:w:0xCF:m
```
or any ATmega32U4-compatible programmer tool and software.
