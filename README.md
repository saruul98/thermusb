# `thermusb`: readout of multiple therm_v3.2 boards for 8*N channel temperature measurements   
Saruul Nasanjargal, AEI (February 2023)

Based on `thermv32usb` by Iouri Bykov.

## Introduction
We have been using Iouri Bykov’s therm_v3.2 boards to measure temperature accurately in many experiments. These are 8-channel boards that implement digital processing using a A3P250 FPGA, and have both RS-232 and USB interfaces for data acquisition.

**Link to the Eagle project file**: http://elektronik.aei.uni-hannover.de/download/1383

The software we have been using until recently to communicate with these boards via USB (`thermv32usb.c`) allowed communication with only one board. This program makes use of the `libftdi` library, which itself makes use of the `libusb` library. Attempts to communicate with a second board by running new instances of the program failed due to the way in which these libraries operate. Reading out multiple devices on the same machine would require a rewrite of the program.

A new development, `thermusb.cpp`, brings the older code into C++ land, and implements communication with an arbitrary number of boards, among other improvements.

## Installation
As of now, the program can only be compiled and ran on Linux (Windows compatibility requires additional effort, perhaps something to consider in the future). These are the dependencies required to install it (and relevant Ubuntu install commands):

1. `libftdi` (library required for interfacing with FTDI devices via USB, `sudo apt install libftdi1-dev
2. `libncurses` (library required for drawing a text interface, `sudo apt install libncurses-dev`)

These libraries have their own dependencies that should be readily installed when using a package manager such as apt.

The program is provided with a generic Makefile. To compile the program, simply run `make all` from the main program directory. This will generate an executable `thermusb`, which you can run from this folder, or copy to `/usr/local/bin` to be able to run it from anywhere. In Ubuntu and other systems, you need super user privileges to open USB devices. Therefore, you might need to run the program as `sudo ./thermusb`.

## Use
When running the program, it will automatically discover all connected FTDI devices, and initiate communication with them. The timestamped temperature data collected from all devices will be written to a dated readout folder (e.g. `readout/2022_02_16_03_05_32/raw_data.txt`).

## Version history and to-do list

v0.1 (2.16.2023): Initial release 

To-do:
- **Error handling**: the program should quit if no devices are found.
- **Exit program after timer runs out**: it might be useful to have the option have the program quit automatically after a specified amount of time.
- **Windows compatibility: text interface**: compiling on Windows requires changes to the basic text interface of the program, because `libncurses` is, to the best of my knowledge, not available on Windows. A simple workaround is to skip the interface alrogether, and just write a basic output to `stdout`. A more advanced solution would be to implement a basic interface using the SFML library, which is multiplatform (https://www.sfml-dev.org/).
- **Windows compatibility: timestamping**: timestamping currently uses `sys/time.h` functions that are not available in Windows. We need to implement timestamping using `windows.h` if we want Windows compatiblity.
- **Accurate timestamping**: timestamping is currently done using `sys/time.h` functions, and not the GPS module in the board, although the required auxilliary functions to do this are implemented. If the absolute accuracy of the timestamps is of concern, we may want to implement GPS-assisted timestamping.