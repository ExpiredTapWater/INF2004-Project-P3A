# INF2004-Project-P3A
**Robotic Car Project - Team P3A**

## Multicore Branch
The code here is meant to be run on the robotic car mounted Pico, and is optimised to make use of SMP provided by FreeRTOS.
Tasks are split according to producer (core_0) and consumers (core_1) with the aim of minimising cross core communication.

### Core_0 (Producer)
| **Task/Function**   | **Priority** | **Remarks**                                                                                          |
|---------------------|--------------|----------------------------------------------------------------------------------------------------------|
| `Wifi`              | High         | UDP server either via Access Point or Hotspot (Hold GP22 while booting)                                       |
| `Heartbeat`         | Low          | Sends telemetry. Currently only sends total packets received to remote    |
| `LED`               | Low          | Blinks the built in LED. Should always be blinking else another task is blocking    |
| `IR`                | -            | NOT YET IMPLEMENTED |
| `Message`           | Med          | Parse received UDP packets and sends them to correct queue |

### Core_1 (Consumer)
| **Task/Function**   | **Priority** | **Remarks**                                                                                          |
|---------------------|--------------|----------------------------------------------------------------------------------------------------------|
| `Motor`             | Low          | Processes commands from core_0 and sets up the L298N module accordingly  |
| `PID`               | Low          | Updates output PWM according to PID algorithm (Will eventually merge with motor task)|
| `LED`               | Low          | Blinks the built in LED. Should always be blinking else another task is blocking    |

### Either (Interrupts)
| **Task/Function**   | **Remarks**                                                                                          |
|---------------------|------------------------------------------------------------------------------------------------------------------------|
| `Encoder`           | Updates pulse duration based on interrupts. Not pinned to any core, but further processing is done in 'Motor' on core_1

### Queues
| **Name**            | **Remarks**                                                                                          |
|---------------------|------------------------------------------------------------------------------------------------------------------------|
| `received_queue`    | All received UDP messgages |
| `commands_queue`    | Parsed messages that are motor commands. One way. Used by core_1 |

### Folder Structure
    Main Folder/
    ├── CMakeLists.txt          # Root CMakeLists.txt (No need to touch)
    ├── main.c                  # Main driver code goes here (Call all your functions in here)
    ├── FreeRTOS-Kernel/        # FreeRTOS kernel directory (No need to touch)
    ├── source/                 # Source folder (Put all your codes here)
        └── CMakeLists.txt      # Update this with whatever new .c file you added
        └── header.h            # Generic headers used by main.c and other source files
        └── blink.c             # Simple code to test that everything is working
        └── motor               # Motor related code
            └── motor.c         # PID and motor control code
            └── motor.h         # Headers used by motor.c
        └── networking          # Code to support network functions
            └── wifi.c          # Contains code to setup the UDP server
            └── wifi.h          # Headers used by wifi.c
            └── lwipopts.h      # Related headers (No need to touch)
        └── sensors             # Code to support network functions
            └── encoder.c       # For encoder functions



