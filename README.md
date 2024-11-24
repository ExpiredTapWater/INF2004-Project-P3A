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
| `Telemetry`         | Equal        | Pico>Pico telemetry. Pulls basic info and spams the other pico    |

## Rough Explaination

### **Setup**
1. Create tasks
    - Check GP22 to decide if to host (AP mode) or connect to exsisting wifi (Hotspot)
        - AP mode has nearly no lag, but is unreliable in lab environment
        - Hotspot mode is slightly laggier but much more reliable
2. Wait for connection from remote
    - IP address is hardcoded, change in wifi.h
3. Perform handshake: waits for single byte 0xFF from remote, replies with "hello" in text
    - Remote only sends single bytes for efficiency. Uses switch statement to map commands (FWD, BACK..) to the bytes (0x01, 0x02..)
4. Start heartbeat, and telemetry task. We are using UDP so we just start spamming regardless of connection
5. Main task starts, see the table above for the tasks. All of them runs concurrently.

### **Non-blocking ultrasonic implementation (for the e-brake component)**
1. Poll sensor using non blocking means
    - Distance measured is a global variable used by a few tasks
    - Use semaphores and interrupt to sync and prevent tasks from reading bad data
2. Provide a global semaphore so any task can check for obstructions (Like a flag, but more efficient)
3. Ensures the "alert" is only sent once, each tasks decides for itself when to clear its own flag

### **Auto switch to line following**
1. Auto Switcher task waits idle using tasknotifytake with max blocking. Once command is given, it will just lookout for the black line.
 When black line is detected, it:
    - safely dumps all pending commands in queues, 
    - resets variables like motor speed, etc
    - disable non essential interrupts
    - suspends non essential tasks
3. Then notify line_following.c to begin.



