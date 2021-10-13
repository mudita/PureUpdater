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

__WARNING__ 
Updater binary or test binary can be run via ***Ecoboot*** bootloader like standard OS binary. 
You need to change ***.boot.json*** for run the updater or updater tests.

### Updater app

#### Configuring 

Application can be configured for the RT1051 using standard CMake file or provided 
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

Then updater catalog will have _PureUpdater_RT.bin_ to be run on rt1051 ready.

##### Hardware tests application - RT1051

Updater also has unit test framework which is able to run directly on the RT1051 platform.
To build it use custom target.

```shell
    cd <build_dir>
    ninja tests
```

Then tests catalog will have _PureUpdater-test.bin_ tests to be run on rt1051 ready.

### Unit tests on PC

Tests which can be written and tested on PC easily. 
See: [unittest/README.md](./unittest/README.md)
