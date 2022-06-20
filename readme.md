 # Pure Updater

## Information
Pure Updater is an update program which is responsible for upgrading the Mudita OS.
It is based on a bare metal program loaded by Ecoboot.
Pure Updater has its own hardware abstraction layer (HAL) which supports basic functionality needed
by the upgrade process.

HAL provides the following functionality:

* Mini VFS - mini virtual filesystem with support for littlefs and VFAT partitions. 

    After mounting the filesystem, files can be accessed by the standard `<stdio>`/`<unistd>` interface; 
    also extra functions from HAL/VFS can be used directly.

* Delay - minimal time support

    - get_ujiffies() - number of ticks (ms) from system startup
    - msleep(int ms) - busy wait number of ms


* Keyboard - reads keys from the phone keypad 

* Logging - via the standard printf() or stdout, stderr stream to the USART serial console

* System - system runtime for a single threaded application

- When the main function returns using a zero or negative code, the application stops.

```c
    // stop application and exit
    int main(int argc, char** argv)
    {
        return 0;
    }
```
- When the main function returns with a positive error code, the application will restart the system.
```c
    // exit and restart the system
    int main(int argc, char** argv)
    {
        return 1;
    }
```
- If you want to exit also from another functions, the standard exit() function can be used.
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
            //! Terminate and stop the application
            if(need_stop) {
                exit(0);
            }
        }
```

## How to build

__WARNING__ 
An updater or a test binary can be run via the ***Ecoboot*** bootloader like a standard OS binary. 
You need to change ***.boot.json*** for running the updater or updater tests.

### Updater app

#### Configuring 

The application can be configured for RT1051 using a standard CMake file or the provided 
_configure.sh_ script.

##### Standard CMake
```shell
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
```

##### configure.sh

```shell
    ./configure.sh <product> <target> <build_type> [other cmake options]
```
Example for Pure: 
```shell
    ./configure.sh pure rt1051 Debug
```
To check available options use:
```shell
    ./configure.sh --help
```

_build-[product]-[target]-[build_type]_ directory will be created.

#### Building

##### Application

```shell
    cd <build_dir>
    ninja 
```

Then updater catalog will have _PureUpdater_RT.bin_ ready to be run on RT1051.

##### Hardware tests application - RT1051

The updater has also a unit test framework which is able to run directly on the RT1051 platform.
To build it, use a custom target.

```shell
    cd <build_dir>
    ninja tests
```

Then the tests catalog will have _PureUpdater-test.bin_ tests ready to be run on RT1051.

### Unit tests on a PC

Tests which can be written and tested on a PC easily. 
See: [unittest/README.md](./unittest/README.md)
