# INF2004-Project-P3A
**Robotic Car Project - Team P3A**

### Main Code
- Pico (Robotic Car) - 'Multicore' branch [here](https://github.com/ExpiredTapWater/INF2004-Project-P3A/tree/Multicore)
- ESP32/Pico (Remote) - 'Remote' branch [here](https://github.com/ExpiredTapWater/INF2004-Project-P3A/tree/Remote)
- Pico only (Telemetry) - 'Telemetry' branch [here](https://github.com/ExpiredTapWater/INF2004-Project-P3A/tree/Telemetry)

### FreeRTOS Template Source
We followed [This guide](https://learnembeddedsystems.co.uk/freertos-on-rp2040-boards-pi-pico-etc-using-vscode)

# Multicore Branch
Explaination of each FreeRTOS task

| **Task/Function**   | **Priority** | **Remarks**                                                                                          |
|---------------------|--------------|----------------------------------------------------------------------------------------------------------|
| `Wifi`              | High         | UDP server either via Access Point or Hotspot (Hold GP22 while booting)                                       |
| `Heartbeat`         | Low          | Pico>Remote telemetry. Currently only sends total packets received to remote    |
| `LED`               | Low          | Blinks the built in LED. Should always be blinking else another task is blocking. Also blinks when packet is received    |
| `GPIO`              | Low          | Same as LED, but is pinned to the other core. Both should always be blinking    |
| `Message`           | Med          | Parse received UDP packets and sends them to correct queue |
| `Task Manager`      | Low          | Manual switches between remote and line and other debug modes|
| `Auto Switcher`     | Low          | Idle until notified. Switches between remote and line following automatically   |
| `Motor`             | Equal        | Processes commands from core_0 and sets up the L298N module accordingly  |
| `PID`               | Equal        | Updates output PWM according to PID algorithm (Will eventually merge with motor task)|
| `LED`               | Equal        | Blinks the built in LED. Should always be blinking else another task is blocking    |
| `Ultrasonic`        | Equal        | Polling and on-demand reading. Uses semaphores to sync and override motor when obstructed    |
| `LineTask`          | Equal        | Idle until notified, will override motor controls and follow the line    |
| `Barcode`           | Equal        | Barcode related stuff here, always running regardless of manual or line following    |
| `Telemetry`         | Equal        | Pico>Pico telemetry. Pulls basic info and spams the other pico    |

### Folder Structure
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
# Remote Branch
Explaination of each FreeRTOS task

| **Task/Function**   | **Priority** | **Remarks**                                                                                          |
|---------------------|--------------|----------------------------------------------------------------------------------------------------------|
| `Wifi`              | Equal        | Connect to car either via Access Point or Hotspot                                   |
| `Display`           | Equal        | Draw UI element based on current fsm state    |
| `Accelerometer`     | Equal        | Reads and formats values to commands    |
| `LED`               | Equal        | Blinks LED at constant rate. If it stops then something went wrong    |
| `Message Handler`   | Equal        | Parse received UDP packets and formats them to be displayed |
| `Send Commands`     | Equal        | Sends byte packet of command|

### Folder Structure
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
            
## Remote Demo
### Boot
![Boot](https://raw.githubusercontent.com/ExpiredTapWater/INF2004-Project-P3A/refs/heads/main/Images/Boot.gif "Boot")

Select between two hard-coded connection options. Pressing GP21 to start sets the neutral position for the accelerometer

### Main UI
![Main](https://raw.githubusercontent.com/ExpiredTapWater/INF2004-Project-P3A/refs/heads/main/Images/Main.gif "Main")

- Crosshair will move according to the direction of command sent to the car, with magnitude indicating speed
- 'o' indicates heartbeat packet received from car
    - 'S' = Packets sent by remote
    - 'R' = Packets received by car
      
### Extra
![Others](https://raw.githubusercontent.com/ExpiredTapWater/INF2004-Project-P3A/refs/heads/main/Images/Others.gif "Others")

- Disable: No commands will be send. Shown with a large 'X'.
- Remote mode: Overrides line following and returns control to user
- Line Follow: Marked with 'A' at top left. Allows user to continue moving the car, but takes over when line is detected

<img src="https://raw.githubusercontent.com/ExpiredTapWater/INF2004-Project-P3A/refs/heads/main/Images/Telemetry.jpg" width="385px" align="center">

- Each message sent by the car to telemetry is duplicated on the remote for convenience
    - Shows messages received sequentially. Either barcodes or ultrasonic readings

            
# Telemetry Branch
Explaination of each FreeRTOS task

| **Task/Function**   | **Priority** | **Remarks**                                                                                          |
|---------------------|--------------|----------------------------------------------------------------------------------------------------------|
| `Wifi`              | Equal        | Connect to car either via Access Point or Hotspot                                     |
| `LED`               | Equal        | Blinks LED at constant rate. If it stops then something went wrong    |
| `Message Handler`   | Equal        | Parse received UDP packets and formats them to be displayed |

### Telemetry Folder Structure
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




