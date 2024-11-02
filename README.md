## Remote Branch
The code here is meant to be run on an ESP32 flashed with MicroPython firmware. It integrates the MPU6050 sensor along with an SSD1306 OLED display
and functions as the companion device to the robotic car. It heavily utilises asynchronous code to handle the different aspects required.

### Rough Explaination of Implementation

**Setup**
1. Defines all required stuff
    - Hardcoded wifi credentials
    - Global variables
    - Queue object for threadsafety
2. Setup hardware
    - Setup 2 hardware buttons
        - Setup button interrupt for menu navigation
    - Initalise display
    - Initalise accelerometer
3. Display connection selection screen
Allows users to selected either a hotspot or direct (AP mode) connection to the Pico
4. Swap button interrupt for mode (Manual/Line-following) selection
5. Setup selected wifi connection
    - Display progress (raise error if failed, hard blocks till restart)
    - Connect to wifi
    - Create socket/listener
6. Perform manual handshake with Pico (We are using UDP for them speedz)
7. Show calibration screen. Upon button push sets current readings as neutral position

**Main Loop**
1. Receives heartbeat from Pico, containing (telemetry I guess) total recieved packets (1/s)
2. Reads accelerometer values
    - Helper function outputs 3 discrete states for X & Y (High, Low, Neutral)
    - *note: Z axis not used*
3. Update main UI with values
    - Draws crosshair to represent tilt for X and Y axis, and heartbeat indicator
    - Print total sent, received packets and current mode
4. If button is pushed:
    1. Format accelerometer values into binary-like string (8 Bytes)
    2. Get the actual binary representation of that string (1 Byte)
    3. Send that single byte via UDP (Max efficiency yo), increment counter.
    4. spam 3 stop commands once button is released

### Folder Structure
    Main Folder/
    ├── main.py                 # Main driver code
    ├── libraries/              # Source folder (Put all your codes here)
        └── accelerometer.py    # Contains higher level functions for interfacing with the MPU6050
        └── display.py          # Functions to support a basic UI
        └── MPU6050.py          # Driver for the accelerometer sensor      
        └── networking.py       # higher level networking functions for communications
        └── SSD1306.py          # Driver for the SSD1306 OLED display


