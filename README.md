# Raven Collection And Development Library (RavCADL_MCU_ForSTM32)

Raven Collection And Development Library is a personal embedded C library collection for STM32-based projects.

The purpose of this library is to organize reusable peripheral drivers into a clean layered architecture. The structure is inspired by common embedded software architecture and FSP-like driver organization, where application code does not directly control low-level hardware details.

This project currently targets STM32 HAL-based projects and is designed to be easy to port across STM32 families.

---

## Architecture Overview

The RAV library is designed around the following layered architecture:

```text
+-------------------------------------------------------------------------------+
| Visible OS Layer (vOS)                                                        |
+-------------------------------------------------------------------------------+
| Task Layer                                                                    |
+-------------------------------------------------------------------------------+
| Application Layer (APP)                                                       |
+-------------------------------------------------------------------------------+
| Driver Layer                                               | Complex Device   |
|  +----------------+  +----------------+ +----------------+ | Driver (CDD)     |
|  | Calculation    |  | Device         | | Module Driver  | |                  |
|  | Driver (CD)    |  | Driver (DD)    | | (MD)           | |                  |  
|  +----------------+  +----------------+ |                | |                  |   
|  +--------------------------------------+                | |                  |
|  |                                                       | |                  |
|  |                                                       | |                  |
|  +-------------------------------------------------------+ |                  |             
+-------------------------------------------------------------------------------+
| HAL - Hardware Abstraction Layer                                              |
+-------------------------------------------------------------------------------+
| REG - Register Layer                                                          |
+-------------------------------------------------------------------------------+
| Hardware                                                                      |
+-------------------------------------------------------------------------------+
```

The main goal is to keep the application layer clean and hardware-independent as much as possible.

Instead of writing HAL calls directly inside the application, each hardware feature is wrapped inside a RAV driver object and controlled through public APIs.

---

## Layer Description

### 1. Visible OS Layer (vOS) 

This is the highest level of the system.

It can represent:

- Bare-metal super loop
- RTOS task scheduler
- User-visible system behavior
- Main control flow

This layer should not directly manipulate hardware registers or HAL peripherals.

---

### 2. Task Layer

The task layer contains periodic or event-driven jobs.

Examples:

- Speed measurement task
- LCD update task
- Servo control task
- Sensor polling task
- Communication task

Tasks call application APIs or driver APIs to perform work.

---

### 3. Application Layer (APP)  

The application layer contains project-specific logic.

Examples:

- Speed gun application
- Obstacle detection logic
- Motor control logic
- Display formatting logic
- System state machine

This layer may combine multiple drivers together.

For example, a speed gun application may use:

- Two ultrasonic sensors
- One LCD display
- One servo motor
- Application-defined speed calculation

---

### 4. Driver Layer

The driver layer contains reusable RAV drivers.

Drivers are divided conceptually into:

#### Calculation Driver (CD)

Calculation Driver is a reusable software layer that contains calculation methods, mathematical processing, or algorithmic logic used by the application.

Unlike Module Driver or Device Driver, Calculation Driver does not directly control hardware. Instead, it receives input data from the application or other drivers, processes that data using a defined algorithm, and returns calculated results in a consistent format.

In the RAV architecture, the Calculation Driver helps keep mathematical formulas and processing logic separated from the main application code. This makes the application easier to read, test, reuse, and maintain.

Calculation Driver may be used together with other drivers.

Typical responsibilities of a Calculation Driver include:

- Mathematical formula implementation
- Unit conversion
- Sensor data conversion
- Signal processing
- Filtering
- Calibration calculation
- Linear interpolation
- Lookup table processing
- Speed, distance, angle, or position calculation
- Error compensation
- Threshold checking
- Algorithm state management

Examples of Calculation Drivers include:

- Speed calculation driver
- Distance conversion driver
- Moving average filter driver
- Low-pass filter driver
- PID calculation driver
- Sensor calibration driver
- ADC-to-voltage conversion driver
- Temperature conversion driver
- Encoder position calculation driver
- Battery percentage estimation driver

#### Device Driver (DD)

Drivers for a specific device or IC.

Examples:

- ...
- ...
- ...

#### Module Driver (MD)

The Module Driver is a middleware-like layer that is closer to OSI Layer 2, the Data Link Layer.

In the RAV architecture, this layer is responsible for defining and handling communication data frames between low-level communication drivers and the application layer. It does not only transmit raw bytes, but also defines how data is packed, validated, decoded, and mapped into meaningful information. However, depending on the protocol design, the Module Driver may also contain some application-specific behavior, such as mapping command IDs or payload data into application-level meanings.

Typical responsibilities of a Module Driver include:
- Frame format definition
- Message ID or command ID handling
- Payload length management
- Payload encoding and decoding
- Checksum or CRC validation
- Transmit and receive state machines
- Timeout handling
- Mapping received payload data into application-level signals or parameters

Examples:
- RS-232
- RS-485
- CAN
- LIN
- Modbus over UART/RS-485
- Custom UART protocol
- Custom CAN frame protocol

### 5. Complex Device Driver

A higher-level driver that combines multiple lower-level modules.

Examples:

- ...
- ...
- ...

---

### 5. HAL Layer

The HAL layer is provided by STM32Cube HAL.

RAV drivers use STM32 HAL APIs internally, such as:

- `HAL_I2C_Master_Transmit`
- `HAL_TIM_PWM_Start`
- `HAL_TIM_IC_Start_IT`
- `HAL_GPIO_WritePin`
- `HAL_Delay`

The user is expected to configure STM32 peripherals using STM32CubeMX or manually before using the drivers.

---

### 6. REG Layer

The register layer is the lowest software layer.

It represents direct MCU peripheral registers.

In most cases, RAV drivers do not access registers directly. They use STM32 HAL macros and APIs instead.

---

## Library Structure

Recommended project structure:

```text
RAV/
├── Common/
│   ├── RAV_Common_Define.h
│   └── RAV_Common_Porting.h
│
├── Drivers/
│   ├── LCD_I2C/
│   │   ├── RAV_LCD_I2C.c
│   │   └── RAV_LCD_I2C.h
│   │
│   ├── RC_Servo/
│   │   ├── RAV_RC_SERVO.c
│   │   └── RAV_RC_SERVO.h
│   │
│   └── Ultrasonic/
│       ├── RAV_ULTRASONIC_HYSRF05.c
│       └── RAV_ULTRASONIC_HYSRF05.h
│
└── README.md
```

Inside an STM32CubeIDE project, the files can be placed like this:

```text
Core/
├── Inc/
│   ├── RAV_Common_Define.h
│   ├── RAV_Common_Porting.h
│   ├── RAV_LCD_I2C.h
│   ├── RAV_RC_SERVO.h
│   └── RAV_ULTRASONIC_HYSRF05.h
│
└── Src/
    ├── RAV_LCD_I2C.c
    ├── RAV_RC_SERVO.c
    └── RAV_ULTRASONIC_HYSRF05.c
```

---

## Common Files

### RAV_Common_Define.h

This file defines common types and return codes used by all RAV APIs.

The main return type is:

```c
rav_err_t
```

Example return values:

```c
RAV_SUCCESS
RAV_ERR_INVALID_POINTER
RAV_ERR_INVALID_ARGUMENT
RAV_ERR_TIMEOUT
RAV_ERR_WRITE_FAILED
RAV_ERR_IN_USE
RAV_ERR_NOT_ENABLED
```

All public RAV APIs return `rav_err_t` where possible, allowing the application layer to detect and handle errors consistently.

---

### RAV_Common_Porting.h

This file selects the correct STM32 HAL header depending on the STM32 family.

Supported family macros include:

```c
STM32F1xx
STM32F4xx
STM32F7xx
STM32H7xx
```

Example:

```c
#if defined(STM32F1xx)
 #include "stm32f1xx_hal.h"
#elif defined(STM32F4xx)
 #include "stm32f4xx_hal.h"
#endif
```

This allows the same RAV driver code to be reused across different STM32 families with minimal changes.

---

## Available Drivers
- DD_01_LCD_1602_2004_I2C
- DD_02_RC_SERVO
- DD_03_ULTRASONIC_HYSRF05

## Coding Style

The RAV library follows these style rules:

- Object-based C driver design
- One object instance per hardware module
- Public APIs return `rav_err_t`
- Driver files are separated into `.h` and `.c`
- Header files contain object definitions and public API prototypes
- Source files contain private helper functions and public API implementations
- STM32 HAL access is wrapped inside driver APIs
- Application code should not directly control low-level driver internals

---

## Design Goals

The main design goals of the RAV STM32 Driver Layer are:

- Reusable embedded drivers
- Cleaner application code
- Consistent API style
- Better error handling
- Easier debugging
- Easier porting between STM32 families
- Clear separation between application logic and hardware control

---

## Current Status

Current implemented drivers:

- RAV Common Define
- RAV Common Porting
- RAV LCD I2C
- RAV RC Servo
- RAV Ultrasonic HY-SRF05

Planned future drivers may include:

- GPIO abstraction driver
- UART communication driver
- Button driver
- Buzzer driver
- Motor driver
- Sensor module drivers
- Complex device drivers

---

## License

This project is developed for learning, research, and personal STM32 embedded projects.

You may use, modify, and extend it for your own embedded projects.

---

## Author

Raven Pham

Raven Collection And Development Library
