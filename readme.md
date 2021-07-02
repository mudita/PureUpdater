 # Pure updater

## Information
Pure updater is an update program which is responsible for upgrade the MUDITA OS.
It is based on the bare metal program loaded by the ecoboot
Pure updater has own hardware abstraction layer (HAL) with supports basic functionality needed
by the upgrade process.

HAL provides following functionality:

* Mini VFS - mini virtual filesystem with support littlefs and vfat partitions. 

    After mount files can be accessible by the standard <stdio> <unistd> filesystem interface, 
    also extra functions from hal/vfs cam be used directly

* Delay - Minimal time support

    - get_ujiffies() - number of ticks (ms) from system startup
    - msleep(int ms)  - busy wait amount of ms


* Keyboard  - Read keys from the phone keypad 

* Logging - via the standard printf() or stdout, stderr stream to the Usart serial console

* System - Other system functionality 

    - system_reset() - restart the system
    - exit() or return from main - also should restart the system in non debug mode


## How to build

Application can be build for the RT1051 using standard CMAKE file
```shell
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    ninja 
```
After build _PureUpdater.bin_ should be generated. 

Updater also has unit test framework which is able to run directly on the RT1051 platform. Tests can be build by invoke target: 
```shell 
    cmake tests
```
Test should generate _PureUpdater-test.bin_ binary.


Updater binary or test binary can be run via *Ecoboot* bootloader like standard OS binary.
You need to change .boot.json for run the updater or updater tests.
