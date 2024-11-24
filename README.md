# INF2004-Project-P3A
**Robotic Car Project - Team P3A**

## Main Code
- Pico (Robotic Car) - 'Multicore' branch [here](https://github.com/ExpiredTapWater/INF2004-Project-P3A/tree/Multicore)
- ESP32/Pico (Remote) - 'Remote' branch [here](https://github.com/ExpiredTapWater/INF2004-Project-P3A/tree/Remote)
- Pico only (Telemetry) - 'Telemetry' branch [here](https://github.com/ExpiredTapWater/INF2004-Project-P3A/tree/Telemetry)

## FreeRTOS Template Source
We followed [This guide](https://learnembeddedsystems.co.uk/freertos-on-rp2040-boards-pi-pico-etc-using-vscode)
   
## Robotic Car Folder Structure
    Main Folder/
    ├── CMakeLists.txt              # Root CMakeLists.txt
    ├── main.c                      # Main driver code goes here
    ├── FreeRTOS-Kernel/            # FreeRTOS kernel directory
    ├── source/                     # Source folder
        └── CMakeLists.txt          # Update this with whatever new .c file you added
        └── header.h                # Generic headers used by main.c and other source files
        └── io                      # Folder containing all IO related stuff
            └── barcodes.c          # Barcode logic
            └── blink.c             # For custom LED flashing patterns. Used for basic IO
            └── io_handler.c        # Handles all IO operations, contains task manager and also buzzer stuff
            └── line_following.c    # line following algorithms
            └── station1.c          # No longer used station 1 code.     
        └── motor                   # Motor related code
            └── motor.c             # PID and motor control code
            └── motor.h             # Headers used by motor.c
            └── commands.h          # Maps motor commands to 1 byte commands
        └── networking              # Code to support network functions
            └── wifi.c              # Contains code to setup the UDP server
            └── wifi.h              # Headers used by wifi.c
            └── lwipopts.h          # Related headers (No need to touch)
        └── sensors                 # Sensor related code
            └── encoder.c           # For encoder functions
            └── infrared.c          # Callbacks for infrared stuff
            └── interrupts.c        # Not a sensor but consolidates the setup of all GPIO interrupts and callbacks
            └── ultrasonic.c        # Code for the HC-SR04 ultrasonic sensor
            └── sensor.h            # Headers used for most sensors

## Remote Folder Structure
    Main Folder/
    ├── CMakeLists.txt              # Root CMakeLists.txt
    ├── main.c                      # Main driver code goes here
    ├── FreeRTOS-Kernel/            # FreeRTOS kernel directory
    ├── source/                     # Source folder
        └── CMakeLists.txt          # Update this with whatever new .c file you added
        └── header.h                # Generic headers used by main.c and other source files
        └── io                      # Folder containing all IO related stuff
            └── blink.c             # For custom LED flashing patterns. Used for basic IO
            └── io_handler.c        # Handles all IO operations, handles button, interrupts and fsm
        └── display                 # UI related stuff for use on SSD1306 display
            └── display.c           # Draws UI element
            └── font.h              # Generic font
            └── ssd1306.c           # Driver for the display
            └── ssd1306.h           # Header files for driver
        └── networking              # Code to support network functions
            └── wifi.c              # Contains code to connect to the car
            └── wifi.h              # Headers used by wifi.c
            └── lwipopts.h          # Related headers (No need to touch)
        └── sensors                 # Sensor related code
            └── mpu6050.c           # Driver for the accelerometer
            └── mpu6050.h           # Header files for driver
            └── sensor.c            # Maps sensor to commands

## Telemetry Folder Structure
    Main Folder/
    ├── CMakeLists.txt              # Root CMakeLists.txt
    ├── main.c                      # Main driver code goes here
    ├── FreeRTOS-Kernel/            # FreeRTOS kernel directory
    ├── source/                     # Source folder
        └── CMakeLists.txt          # Update this with whatever new .c file you added
        └── header.h                # Generic headers used by main.c and other source files
        └── io                      # Folder containing all IO related stuff
            └── blink.c             # For custom LED flashing patterns. Used for basic IO
            └── io_handler.c        # Handles basic message receive and display on serial monitor
        └── networking              # Code to support network functions
            └── wifi.c              # Contains code to connect to the car
            └── wifi.h              # Headers used by wifi.c
            └── lwipopts.h          # Related headers (No need to touch)




