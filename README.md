# INF2004-Project-P3A
**Robotic Car Project - Team P3A**

## Multicore Branch
The code here is meant to be run on the robotic car mounted Pico. 

Tasks are split according to producer (core_0) and consumers (core_1) with the aim of minimising cross core communication.

| **Task Name**       | **Core Assignment** | **Priority** | **Remarks**                                                                                          |
|---------------------|---------------------|--------------|----------------------------------------------------------------------------------------------------------|
| `Wifi`              | Core 0              | High         | UDP server either via Access Point or Hotspot (Hold GP22 while booting)                                       |
| `Ultrasonic`        | Core 0              | Low          | -    |
| `IR`                | Core 0              | Low          | NOT YET IMPLEMENTED                      |
| `IO_Handler`        | Core 0              | Med          | Parse received UDP packets and sends them to correct queue                           |
| `Motor`             | Core 1              | Low         | Manages LED indication, set to a lower priority as it does not require immediate execution.               |
| `PID`               | Core 1              | High         | Dedicated to receiving network packets to avoid interruption from other tasks on Core 1.                  |

### Folder Structure
    Main Folder/
    ├── CMakeLists.txt      # Root CMakeLists.txt (No need to touch)
    ├── main.c              # Main driver code goes here (Call all your functions in here)
    ├── FreeRTOS-Kernel/    # FreeRTOS kernel directory (No need to touch)
    ├── source/             # Source folder (Put all your codes here)
        └── CMakeLists.txt  # Update this with whatever new .c file you added
        └── header.h        # Update this with your function prototypes before calling in main.c
        └── blink.c         # Simple code to test that everything is working (No need to touch)
        └── wifi.c          # Contains code to setup the TCP server (No need to touch)
        └── networking      # Contains templated DHCP server code (No need to touch)
            └── dhcpserver.c    # Code to give out IP address when hosting (No need to touch)
            └── dhcpserver.h    # Related headers (No need to touch)
            └── lwipopts.h      # Related headers (No need to touch)


