
# teres1-debug

teres1-debug and debug_switch.sh are DIY-TERES1 utilities for turning debug on or off 


Usage: All commands must be executed as superuser

- switch on/off debug for current session only until reboot
```bash
sudo teres1-debug on
```
or
```bash
sudo teres1-debug off
```

- switch on/off debug permanently
```bash
sudo debug_switch.sh on
```
or
```bash
sudo debug_switch.sh off
```



How to download and install:
```bash
git clone https://github.com/d3v1c3nv11/teres1-debug.git
cd teres1-debug
make
sudo make install
```
destination path for installing is /usr/local/sbin, to change - edit Makefile


Jack connection:
```bash
TX   RX
 |   |    
=== == ====|||||||||----------------------------
         |
        GND
```

## VERY IMPORTANT!!! 
## Use 3.3V level serial adapter! Consider our TERES-USB-DEBUG cable: https://www.olimex.com/Products/DIY-Laptop/KITS/TERES-USB-DEBUG/
## DO NOT USE RS232 LEVEL ADAPTERS!


