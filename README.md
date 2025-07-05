# FreeRTOS-HW-Verification
This is a revised version of my bare-metal [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/), implemented using FreeRTOS in the STM32's code and multithreaded functionality in the PC's code.

## Content
1. [What's new](#whats-new)
2. [Project Overview](#project-overview)
3. [System Requirements](#system-requirements)
4. [Project Structure](#project-structure)
5. [Usage](#usage)



## What's new
### STM32
1. The bare-metal raw API LWIP stack was upgraded to FreeRTOS (CMSIS v2) and LWIP netconn API.
2. Tests run concurrently now.
3. Timer test was updated: better DMA complete callback and added error tolerance due to the less deterministic nature of multitasking systems. 

### PC
1. A new multithreading code was added ([FILES_FOR_PC/multi_threaded](https://github.com/LeahShl/FreeRTOS-HW-Verification/tree/main/FILES_FOR_PC/multi_threaded)).
2. The [single-threaded](https://github.com/LeahShl/FreeRTOS-HW-Verification/tree/main/FILES_FOR_PC/single_threaded) code was updated to receive packets from the now multitasking STM32 system.

NOTE: Usage and database both stayed the same! The original version, the new single-threaded version and the multithreaded version all can log and export from the same database.

## Project Overview
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

## System Requirements
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

## Project Structure
Key:
- regular file
- \[Directory\]

```
.
├── [FILES_FOR_PC]                     # PC server files
│   ├── [debug_tools]
│   │   └── dummy_server.c             # Mock server to test main.c in abscence of a UUT
│   │
│   ├── [multi_threaded]               # Multithreaded version of PC server
│   │   ├── main.c                     # Multithreaded program's main file
│   │   ├── Makefile                   # Compile with GNU make
│   │   ├── mthw_tester                # Executable name
│   │   ├── tests_db.c                 # SQLite database source file
│   │   ├── tests_db.h                 # SQLite database header file
│   │
│   ├── [single_threaded]              # Single-threaded version of PC server
│   │   ├── hw_tester                  # Executable name
│   │   ├── main.c                     # Single-threaded program's main file
│   │   ├── Makefile                   # Compile with GNU make
│   │   ├── tests_db.c                 # SQLite database source file
│   │   └── tests_db.h                 # SQLite database header file
│   └── usage_example.sh               # Usage examples for PC server
│   
├── README.md                          # <--- This file
│   
└── [STM32CUBE_project]                # STM32 Project files, automatically generated unchanged files skipped
    ├── [Core]
    │   ├── [Src]
    │   │   ├── freertos.c             # FreeRTOS main source file
    │   │   ├── main.c                 # STM32 main file
    ├── FreeRTOS_HW_Verification.ioc   # IOC File with the MX configuration
    |
    └── [UUT]                          # All project files for the UUT are here
        ├── [Inc]
        │   ├── adc_test.h             # ADC test header file
        │   ├── dispatcher.h           # Dispatcher task header file
        │   ├── hw_verif_crc.h         # CRC header file
        │   ├── hw_verif_sys.h         # General system definitions
        │   ├── hw_verif_udp.h         # UDP functionality (UDP tasks here) header file
        │   ├── i2c_test.h             # I2C test header file
        │   ├── spi_test.h             # SPI test header file
        │   ├── timer_test.h           # Timer test header file
        │   └── uart_test.h            # UART test header file
        └── [Src]
            ├── adc_test.c             # ADC test source file
            ├── dispatcher.c           # Dispatcher task source file
            ├── hw_verif_crc.c         # CRC source file
            ├── hw_verif_udp.c         # UDP functionality (UDP tasks here) source file
            ├── i2c_test.c             # I2C test source file
            ├── spi_test.c             # SPI test source file
            ├── timer_test.c           # Timer test source file
            └── uart_test.c            # UART test source file
```

## Usage

### 1. Build PC Program
Multithreaded version:
1. Open terminal and `cd` into `FILES_FOR_PC/multi_threaded`
2. run `make`

Single-threaded version:
1. Open terminal and `cd` into `FILES_FOR_PC/single_threaded`
2. run `make`

NOTE: Each version's make produces a differently named executable. To run the single threaded version use 'hw_tester' and to run the multithreaded version use 'mthw_tester'.

### 2. Setup Network Configuration
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

### 3. Build STM32 Firmware
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

### 4. Setup Hardware
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

### 5. Run Tests
Generally the same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

In the multithreaded version of the PC code, stacked flags will cause tests to run concurrently, i.e:

```
./mthw_verif --all                          # Run all tests concurrently
./mthw_verif --all -n 10                    # Run all test concurrently with specified number of iterations
./mthw_verif -uit -s "SPI"                  # Run uart, i2c, timer test concurrently, then spi test with a custom message
```

## Communication Protocol
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/).

## Testing Methodology
Same as [STM32F746ZG Hardware Verification System](https://github.com/LeahShl/STM32F756ZG_HW_Verification/) except timer test.
### Timer Test
1. TIM2 (advanced timer) is set to count up.
2. TIM6 (basic timer) is set to restart every 10ms.
3. TIM2 count is sampled every time TIM6 restarts, until N_SAMPLES(=10) are retrieved.
4. A single test passes if all samples are spaced 54,000 counts from each other, with a tolerance defaulting to 4000.
5. The peripheral passes the test if all iterations passed.