## Remote Branch
The code here is meant to be run on an ESP32 flashed with MicroPython firmware. It integrates the MPU6050 sensor along with an SSD1306 OLED display
and functions as the companion device to the robotic car. It heavily utilises asynchronous code to handle the different aspects required.

## Rough Explaination of Implementation
1. **Define All Required Configurations**
   - Hardcoded Wi-Fi credentials
   - Global variables
   - Queue object for thread safety

2. **Setup Hardware**
   - **Buttons**: Setup 2 hardware buttons
     - Configure button interrupt for menu navigation
   - **Display**: Initialize display
   - **Accelerometer**: Initialize accelerometer

3. **Display Connection Selection Screen**
   - Allows users to select either:
     - Hotspot mode
     - Direct connection (AP mode) to the Pico

4. **Swap Button Interrupt**: 
   - Update for mode selection (Manual/Line-following)

5. **Setup Wi-Fi Connection**
   - Display progress and raise error if connection fails (blocks until restart)
   - Connect to Wi-Fi
   - Create socket/listener

6. **Perform Manual Handshake with Pico**
   - Use UDP for them speeedz

7. **Show Calibration Screen**
   - Upon button press, set current readings as the neutral position

---

## Main Loop

1. **Receive Heartbeat from Pico**
   - Contains total received packets (1/s)

2. **Read Accelerometer Values**
   - Helper function outputs 3 discrete states for X & Y axes: High, Low, Neutral
   - *Note: Z axis is not used*

3. **Update Main UI**
   - Draw crosshair to represent tilt for X and Y axes
   - Show heartbeat indicator
   - Display total sent and received packets, and current mode

4. **Button Push Handling**
   1. Format accelerometer values into a binary-like string (8 bytes)
   2. Convert that string into an actual binary representation (1 byte)
   3. Send the single byte via UDP, then increment the counter
   4. When button is released, send 3 stop commands


### Folder Structure
    Main Folder/
    ├── main.py                 # Main driver code
    ├── libraries/              # Source folder (Put all your codes here)
        └── accelerometer.py    # Contains higher level functions for interfacing with the MPU6050
        └── display.py          # Functions to support a basic UI
        └── MPU6050.py          # Driver for the accelerometer sensor      
        └── networking.py       # higher level networking functions for communications
        └── SSD1306.py          # Driver for the SSD1306 OLED display


