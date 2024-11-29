# SMARTPROBE Pitot3D device

Smartprobe is an autonomous pitot3d tube that is designed to be mounted on a drone.
It records relative wind speed and angles, stores them on sdcard, and stream data on a serial line.



#### *Disclaimer : software was still in alpha stage when the project development was stopped in fall 2021*



## Installation


### The easy way
use the latest binary found under GitHub Releases

### You really want to compile it ?
### Dependencies installation 


>cross compilation and Perl library for the boardGen.pl tool :

``` shell
sudo apt install build-essential gcc-arm-none-eabi gdb-multiarch \
         default-jre libmodern-perl-Perl \
	 libxml-libxml-Perl libfile-which-Perl \
	 dfu-util
	 
sudo cpan -i String/LCSS.pm
```
> to use bmpflash when flashing with a Black Magic Probe or Criquet Probe
``` shell
sudo usermod -a -G dialout $USER
```
### optional : stm32cubemx install from stmicroelectronics website:
https://www.st.com/en/development-tools/stm32cubemx.html
(boardGen.pl script needs xml MCU description files)
If stm32cubemx is not found, a pregenerated board.h is used, you won't be able to modify pin assignation


### clone of GitHub repository and submodules :
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
plug micro-USB cable (no need to power supply)
``` shell
make BUILD=SIZE dfuflash

```
In swd mode :
plug swd cable (need to power supply smartprobe)
``` shell
make BUILD=SIZE flash
``` 


## Usage

### µSD Card
It is mandatory to install a micro sd card :
- SD should be formatted with FAT32 or ExFAT filesystem.
- configuration is read from SD
- wind data is stored on the SD
- reading/writing SD can be done
  - offline : SD must be removed
  - online : plug the smartprobe via USB (with SD installed) on a computer, and it will be shown like a USB storage device


### Connector J3 at the back
The Molex connector at the back contains  :  
- power in : Vin should be between 7 and 20 Volts
- bootloader mode switch (connect pin5 with pin6, then power up the smartprobe)
  -bootloader mode is to be used to flash the firmware in the smartprobe
- USB to mount the sdcard as a USB storage device
- UART which can be used for multiple purposes (choice is made in configuration file)

 ![Molex connector wiring](/doc/connecteur_smartprobe_v4.png) 

### Parameters
- firmware parameters are read from the SD at start, in a file named ***smartprobe.conf***
- if a file named smartprobe.conf is not found, it will be created, all the fields set at their default value
- the most accurate and up-to-date definitions (with default and range) for each field can be found in source file  
*source/confParameters.hpp*
- here is a list of parameters (details are missing, confParameters.hpp reading is advised)

|Parameter				|                       Description                   	| default 
|-----------				| --------------------------------------------------  	| -----------
|syslog.filename			| name of the file where system events are logged	|syslog
|sensorslog.filename			| name of the file where sensors data are logged	|sensors
|thread.frequency.d_press		| differential pressure sampling frequency  		|100
|thread.frequency.imu			| IMU sampling frequency 				|100
|thread.frequency.stream_console	| frequency of messages on the console			|10
|thread.frequency.transmit_uart		| frequency of messages on serial link			|100
|ahrs.type				| type of AHRS : raw, headless, or complete		|headless_ahrs
|ahrs.output				| ahrs output format : euler or quaternion		|euler
|uart.mode				| no_used or pprz or nmea_in or ubx_in			|pprz
|pprz.message				| pprz msg to stream data : aeroprobe or smartprobe	|aeroprobe
|uart.baud				| Baud rate of the uart in the range 9600..460800	|115200
|canbus.mode				| not yet implemented					|
|canbus.id				| not yet implemented					|
|sensor.barometer.temperatureBias	| temperature bias of the measured exterior air		|-5
|sensor.barometer.lpf			| barometer clock divider    (see LPS33HW datasheet)	|div2
|sensor.barometer.odr			| barometer output data rate (see LPS33HW datasheet)	|50hz
|sensor.d_press.fetched			| diff pressure param fetched : press or press + temp	|P+T
|sensor.d_press.fetchTempFrequency	| diff pressure temp frequency fetching			|1<<
|sensor.imu.accrate			| imu acc fetching rate (see ICM20600 datasheet)	|1khz_bw99
|sensor.imu.gyrorange			| gyro range [250 .. 2000] degrees per seconds		|250_dps
|sensor.imu.accrange			| acc range [2 .. 16] G					|2g
|sensor.imu.estimationLoopDuration_ms	| ahrs calibration duration [10 .. 4000] milliseconds	|500
|airspeed.rho				| estimated (0) or fixed in the range [0.1 .. 2]	|0(force estimation)
|airspeed.calibration.m11		| transformation matrix	from the			|1.15
|airspeed.calibration.m12		| calibration process					|0
|airspeed.calibration.m13		| default values can be used to verify that		|0
|airspeed.calibration.m14		| the smartprobe roughly works, but should not be	|6
|airspeed.calibration.m15		| used to make actual measures				|6
|airspeed.calibration.m21		| 							|0
|airspeed.calibration.m22		| 							|15
|airspeed.calibration.m23		| 							|0
|airspeed.calibration.m24		| 							|0
|airspeed.calibration.m25		| 							|0
|airspeed.calibration.m31		| 							|0
|airspeed.calibration.m32		| 							|0
|airspeed.calibration.m33		| 							|11
|airspeed.calibration.m34		| 							|0
|airspeed.calibration.m35		|		 					|0
|airspeed.calibration.bias.velocity	|bias on air velocity (from calibration)		|0
|airspeed.calibration.bias.alpha	|bias on alfa angle (from calibration)			|0
|airspeed.calibration.bias.beta 	|bias on beta angle (from calibration)			|0
|led.mode				|status or airspeed					|status
|					|							|


## Calibration
Smartprobe should be calibrated in a wind tunnel before use. The calibration matrix values should then be written in the configuration file instead of the default values.

### Calibration procedure :
...TO BE WRITTEN...


## Development tool
there is a SWD connector on the PCB for connecting a debug probe (Black Magic Probe or STlink V2/V3) which permits debugging with gdb and access to a shell console

## Ideas for future developments 
- If external componant (autopilot) stream gps positions and heading, wind in a terrestrial reference frame could be calculated on board
- add USB CDC endoint to the existing USB storage endpoint to output serial console shell via USB
- stream UAVCAN messages on the CAN 2.0B interface
