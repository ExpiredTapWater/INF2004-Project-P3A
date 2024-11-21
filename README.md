## Multicore Branch

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
        └── sensors                 # Code to support network functions
            └── encoder.c           # For encoder functions
            └── infrared.c          # Callbacks for infrared stuff
            └── interrupts.c        # Not a sensor but consolidates the setup of all GPIO interrupts and callbacks
            └── ultrasonic.c        # Code for the HC-SR04 ultrasonic sensor
            └── sensor.h            # Headers used for most sensors



