import time
import network
import uasyncio as asyncio
import usocket as socket
import uselect as select

# Global variables
pico_socket = None

# Connects to a wifi network
def connect_wifi(SSID, PASSWORD, ESP32_IP_ADDRESS, SERVER_IP_ADDRESS, timeout=10):
    
    # Connect in station mode
    station = network.WLAN(network.STA_IF)
    
    # Check if already connected
    if station.isconnected():
        print('Already connected to network')
    
    # Else continue sequence to connect
    else:
        # Enable
        station.active(True)
        
        # Manually configure IP settings
        station.ifconfig((ESP32_IP_ADDRESS, "255.255.255.0", SERVER_IP_ADDRESS, SERVER_IP_ADDRESS))
        station.connect(SSID, PASSWORD)
        print('Connecting to Wi-Fi network...')
        
        # Timeout logic
        start_time = time.time()  # Record the start time
        while not station.isconnected():
            elapsed_time = time.time() - start_time
            if elapsed_time > timeout:
                print(f"Failed to connect to Wi-Fi within {timeout} seconds.")
                return False
            time.sleep(1)
            
        print('IP Address:', station.ifconfig()[0])
        return True

# Creates a UDP socket at the start for future reuse
def create_udp_socket(local_ip='0.0.0.0', port=2004):
    global pico_socket
    try:
        pico_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        pico_socket.setblocking(False)  # Non-blocking mode
        pico_socket.bind((local_ip, port))  # Bind to local IP and port
        print(f"UDP socket bound to {local_ip}:{port} successfully.")
        return True
    except Exception as e:
        print("Error creating or binding socket:", e)
        return False
    
# function to send a udp message  
def send_udp_message(message, IP, PORT=2004):
    try:
        # Send the message
        pico_socket.sendto(message, (IP, PORT))
        #print(f"Sent: '{message}' to {IP}:{PORT}")
        
    except Exception as e:
        print('Error sending UDP message:', e)

# Asynchronous function to continuously receive UDP messages
async def receive_udp_message(queue, buffer_size=64):
    try:
        poller = select.poll()
        poller.register(pico_socket, select.POLLIN)
        print("Started listening for UDP messages...")
        while True:
            events = poller.poll(25)  # Poll every 1000 milliseconds (1 second)
            if events:
                data, addr = pico_socket.recvfrom(buffer_size)
                message = data.decode('utf-8')
                print(f"Received: '{message}' from {addr}")
                # Here, you can add any processing logic for the received message
                
                await queue.put((message))
            # Yield control to allow other tasks to run
            await asyncio.sleep(0)
    except Exception as e:
        print('Error receiving UDP message:', e)
        
# Asynchronous task to manage receiving messages
async def receive_task(queue):
    await receive_udp_message(queue)

# Handshake function
def handshake(IP, buffer_size=16, timeout=5):
    
    # Send hello message to PicoW
    send_udp_message(b'\xFF', IP, 2004)
            
    # Record the start time for timeout
    start_time = time.time()  
    
    try:
        poller = select.poll()
        poller.register(pico_socket, select.POLLIN)
        print("Waiting for handshake")
        
        while True:
            events = poller.poll(100)
            if events:
                data, addr = pico_socket.recvfrom(buffer_size)
                message = data.decode('utf-8')
                
                if message == 'Pico-Hello':
                    print("Handshake received")
                    return True
                
            else: # if no events after each poll, check elapsed time
                elapsed_time = time.time() - start_time
                if elapsed_time > timeout:
                    return False

    except Exception as e:
        print('Error receiving UDP message:', e)
        return False
