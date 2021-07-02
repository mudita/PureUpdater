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

* System - System runtime for single threaded application

    - When main function returns using zero or negative code application will stop
```c
    // Exit and stop application
    int main(int argc, char** argv)
    {
        return 0;
    }
```
    - When main function returns with positive error code application will restart the system
```c
    // Exit and restart the system
    int main(int argc, char** argv)
    {
        return 1;
    }
```
    - If you want to exit also from another functions exit() funcion can be used

```c
#include <stdlib.h>
        void some_function()
        {
            if(need_restart) {
                //! Terminate and restart the system
                exit(1);
            }
        }
        void other_function() 
        {
            //! Terminate and stop application
            if(need_stop) {
                exit(0);
            }
        }
```



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
