# Project Name
Flight Computer Terminal PC Software 

## Description 
Software that allows you to upload flight parameters and dump memory from onboard internal flash 

## Prerequisites
* GCC 
* Make 

## Setup & Compilation 
### Make application.
```
$ cd flight_computer_dump
make all 
```
### Purge all log files. Please save before running this.  
```
$ make purge
```
### Check COM port, and note which one flight controller is connected to.
```
$ sudo dmesg | grep ttyUSB* 
usb 1-5.3: cp210x converter now attached to ttyUSB0
```
### Look for the USB device listed as **cp210x**:
``` 
$ ls /dev/ttyUSB*
/dev/ttyUSB0
```
## Run Application 
### Arguments 
* -d is the path to the USB device 
* -b is the baud rate
* -l is the name of the log file you choose
```bash 
$ ./flight_computer_dump -d /dev/ttyUSB0 -b 921600 -l log-name.bin
```
## View Hex Log File 
### View head of hex file. It is 64 bytes wide, the packet size of each log
```
$ hexdump -v -e '64/1 "%02X " "\n"' logs/log-name.bin | head -n 64
```
### View tail of hex file. 
```
$ hexdump -v -e '64/1 "%02X " "\n"' logs/log-name.bin | tail -n 64
```

