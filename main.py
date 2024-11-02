# Import Stuff
import time
import uasyncio as asyncio
from machine import Pin, Timer

# Helpers
import networking as wifi
import display as display
import accelerometer as accel

# --- Networking Variables ---
SERVER_UDP_PORT = 2004

# Direct Connection
ACCESS_POINT_NAME = 'PicoW-P3A'
ACCESS_POINT_PASSWORD = '12345678'
ACCESS_POINT_PICO_IP = '192.168.4.1'
ACCESS_POINT_ESP32_IP = '192.168.4.2'

# Hotspot Connection
HOTSPOT_NAME = 'CY'
HOTSPOT_PASSWORD = 'wifipassword'
HOTSPOT_GATEWAY_IP = '172.20.10.1'
HOTSPOT_PICO_IP = '172.20.10.2'
HOTSPOT_ESP32_IP = '172.20.10.3'

# --- Hardware ----
OLED_SDA = 22
OLED_SCL = 23
OLED_WIDTH, OLED_HEIGHT = 128, 64
SENSOR_SDA = 32
SENSOR_SCL = 21
TOUCH_BTN = 33
BOOT_BTN = 0

# --- Global Variables ---
PACKETS_SENT = 0
PACKETS_RECV = 0
Button = None
Boot_Btn = None
Calibrated = False
HOTSPOT_MODE = False
PULSE = False

# --- Button Stuff ---
DEBOUNCE = False
DEBOUNCE_DELAY = 250

# --- Mode Selection ----
MODES = ['manual', 'auto']
TOTAL_MODES = len(MODES)
CURRENT_MODE = 0

# Queue object for inter-task communication
class SimpleAsyncQueue:
    def __init__(self):
        self.queue = []
        self.lock = asyncio.Lock()
        self.new_item = asyncio.Event()

    async def put(self, item):
        async with self.lock:
            self.queue.append(item)
            self.new_item.set()

    async def get(self):
        while True:
            async with self.lock:
                if self.queue:
                    item = self.queue.pop(0)
                    if not self.queue:
                        self.new_item.clear()
                    return item
            await self.new_item.wait()

    def task_done(self):
        pass  # Placeholder if needed for compatibility

# Function to setup all required hardware before running main loop
def setup():
    
    global Button, Boot_Btn
    
    # ---- Setup Hardware ----
    Button = Pin(TOUCH_BTN, Pin.IN)
    Boot_Btn = Pin(BOOT_BTN, Pin.IN, Pin.PULL_UP)
    
    # Setup OLED first, so user can get some form of feedback
    if display.init_oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT):
        display.status('boot')
    
    # Setup accelerometer
    if not accel.init_sensor(SENSOR_SDA, SENSOR_SCL):
        display.status('boot-err')
        raise Exception("Accelerometer Error, did you forget to connect it?")
    
    # ---- Select Connection Mode ----
    # Set up an interrupt to handle mode switching
    Boot_Btn.irq(trigger=Pin.IRQ_FALLING, handler=connection_mode_interrupt)
    
    # Allow user to select direct or hotspot connection
    asyncio.run(select_mode())
    
    # Change button function to mode selection
    Boot_Btn.irq(trigger=Pin.IRQ_FALLING, handler=running_mode_interrupt)

    # ---- Setup Connection ----
    # At this point, hardware is ok. Connect to PicoW
    display.status('wifi')
    
    if HOTSPOT_MODE:
        print("Hotspot")
        if not wifi.connect_wifi(HOTSPOT_NAME, HOTSPOT_PASSWORD, HOTSPOT_ESP32_IP, HOTSPOT_GATEWAY_IP):
            display.status('wifi-err')
            raise Exception("Wifi Error, unable to connect")
    else:
        print("Direct")
        if not wifi.connect_wifi(ACCESS_POINT_NAME, ACCESS_POINT_PASSWORD, ACCESS_POINT_ESP32_IP, ACCESS_POINT_PICO_IP):
            display.status('wifi-err')
            raise Exception("Wifi Error, unable to connect")
        
     
    print('Successfully connected to PicoW.')
    
    #raise Exception("Temporary Stop")

    display.status('udp')
    
    # Successfully connected to PicoW, try to create a socket
    if not wifi.create_udp_socket():
        display.status('udp-err')
        
    if HOTSPOT_MODE:
        IP = HOTSPOT_PICO_IP
    else:
        IP = ACCESS_POINT_PICO_IP
        
    if wifi.handshake(IP):
        display.status('ok')
        time.sleep(1)
    else:
        display.status('udp-err')
        raise Exception("Handshake Error, restart and try again!")
   
# Function to allow user to select connection mode
async def select_mode():

    EXIT_COUNTER = 0
    COUNTER = 2.9
    
    while COUNTER >= EXIT_COUNTER:
        
        countdown = str(int(COUNTER-EXIT_COUNTER)+1)
        
        display.connection_ui(
            HOTSPOT_PICO_IP,
            ACCESS_POINT_PICO_IP,
            HOTSPOT_MODE, countdown)
        
        COUNTER -= 0.1
        await asyncio.sleep(0.05)
        
# Function to reset debounce flag        
def reset_debounce(timer):
    global DEBOUNCE
    DEBOUNCE = False
    
# Callback for interrupt
def connection_mode_interrupt(pin):
    
    global DEBOUNCE, HOTSPOT_MODE
    
    if not DEBOUNCE:
        DEBOUNCE = True
        debounce_timer = Timer(-1)
        debounce_timer.init(period=DEBOUNCE_DELAY, mode=Timer.ONE_SHOT, callback=reset_debounce)
        
        # Button Task Below
        HOTSPOT_MODE = not HOTSPOT_MODE
        
# Callback for interrupt
def running_mode_interrupt(pin):
    
    global DEBOUNCE, CURRENT_MODE, PACKETS_SENT
    
    
    if not DEBOUNCE:
        DEBOUNCE = True
        debounce_timer = Timer(-1)
        debounce_timer.init(period=DEBOUNCE_DELAY, mode=Timer.ONE_SHOT, callback=reset_debounce)
        
        # Change to next mode, reset if exceed
        CURRENT_MODE += 1
        if CURRENT_MODE > TOTAL_MODES-1:
            CURRENT_MODE = 0
            
        if MODES[CURRENT_MODE] == 'manual':
            command = '11110001'
            
        elif MODES[CURRENT_MODE] == 'auto':
            command = '11110010'
            
        # Add more possible modes and commands here
        
        if HOTSPOT_MODE:
            IP = HOTSPOT_PICO_IP
        else:
            IP = ACCESS_POINT_PICO_IP
        
        # Send command
        command_byte = int(command, 2).to_bytes(1, 'big')
        wifi.send_udp_message(command_byte, IP)
        PACKETS_SENT += 1
        

# Function to set centre position
def reset_centre():
    
    # Display calibration page
    display.calibrate()

    while True:
        if not Button.value():
            accel.calibrate()
            return

# Task to receive UDP packets
async def receive_task(queue):
    asyncio.create_task(wifi.receive_task(queue))
    while True:
        await asyncio.sleep(0)
    
# Task to process receive UDP packets from queue
async def process_messages(queue):
    global PACKETS_RECV, PULSE
    while True:
        message = await queue.get()         
        PACKETS_RECV = int(message)
        PULSE = not PULSE
        
        # Add your processing logic here
        #print(f"Processing message: '{message}'")
        
        queue.task_done()

def format_command(F, B, L, R, FB_MAG, LR_MAG):
    
    # Simply diagonal movements by removing magnitudes
    # Leaves only 4 diagonal movements instead of 8.
    # Total of 12 possible movement directions/magnitude
    
    a = FB_MAG if F and not (L or R) else 0
    b = FB_MAG if B and not (L or R) else 0
    c = LR_MAG if L and not (F or B) else 0
    d = LR_MAG if R and not (F or B) else 0
            
    command = f'{F}{B}{L}{R}{a}{b}{c}{d}'
    command_byte = int(command, 2).to_bytes(1, 'big')

    return command_byte

async def main_task():
    
    global PACKETS_SENT
    send_final_stop = False
    
    if HOTSPOT_MODE:
        IP = HOTSPOT_PICO_IP
    else:
        IP = ACCESS_POINT_PICO_IP
    
    while True:
        
        # Get live accelerometer data
        F, B, L, R, FB_MAG, LR_MAG = await accel.get_direction()
        
        # Update UI with live data
        display.main_ui(F, B, L, R, FB_MAG, LR_MAG, PACKETS_SENT, PACKETS_RECV, PULSE, CURRENT_MODE)
        
        # If the send button has been pushed
        if(not Button.value()):
            
            # Ensure flag to spam stop is set, then format command to single byte
            send_final_stop = True
            command = format_command(F, B, L, R, FB_MAG, LR_MAG)
            
            # If command is stop, don't send anything
            if command != b'\x00':
                
                wifi.send_udp_message(command, IP)
                PACKETS_SENT += 1
            
        elif send_final_stop:
            send_final_stop = False
            
            # Spam stop command.
            wifi.send_udp_message(b'\x00',IP)
            await asyncio.sleep(0.01)
            wifi.send_udp_message(b'\x00',IP)
            await asyncio.sleep(0.01)
            wifi.send_udp_message(b'\x00',IP)
            PACKETS_SENT += 3
            
        await asyncio.sleep(0)
        
# Main Task    
async def main():
    
    # Create queue for incoming messages
    MESSAGE_QUEUE = SimpleAsyncQueue()
    
    # Setup task to receieve messages
    udp_task = asyncio.create_task(wifi.receive_task(MESSAGE_QUEUE))
    
    # Setup task to handle received messages
    message_task = asyncio.create_task(process_messages(MESSAGE_QUEUE))
    
    # Setup task to print sensor tilt
    sensor_task = asyncio.create_task(main_task())
    
    # Run both tasks concurrently
    await asyncio.gather(udp_task, message_task)

setup()
reset_centre()
asyncio.run(main())
