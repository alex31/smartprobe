# SMARTPROBE PITOT3D device

Smartprobe is an autonomous pitot3d tube that is designed to be mount on a drone.
It records relative wind speed and angle, store it on sdcard, and stream data on a serial line.
If a gps or an autopilot give furnish it gps positions, it can calculate on board wind
in a terrestrial reference frame.

### Dependancies installation 


>cross compilation and perl library for the boardGen.pl tool :

``` shell
sudo apt install build-essential gcc-arm-none-eabi gdb-multiarch \
         default-jre libmodern-perl-perl \
	 libxml-libxml-perl libfile-which-perl \
	 dfu-util
	 
sudo cpan -i String/LCSS.pm
```
> to use bmpflash whien flashing with a black magic probe or criquet probe
``` shell
sudo usermod -a -G dialout $USER
```
### optionnal : stm32cubemx install from stmicroelectronics website:
https://www.st.com/en/development-tools/stm32cubemx.html
(boardGen.pl script need xml MCU description files)
If stm32cubemx is not found, a pregenerated board.h is used, you won't be able to modify pin assignation


### clone of github repository and submodules :
``` shell
git clone --recursive https://github.com/alex31/smartprobe.git
cd smartprobe
git submodule update --init --recursive
```

### build firmware in release mode
``` shell
make clean; make BUILD=SIZE
```

### flash firmware in release mode
In dfu mode :
plug micro-usb cable (no need to power supply)
``` shell
make BUILD=SIZE dfuflash

```
In swd mode :
plug swd cable (need to power supply smartprobe)
``` shell
make BUILD=SIZE flash

```
