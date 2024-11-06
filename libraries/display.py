import ssd1306
import gfx
from machine import Pin, SoftI2C, Timer

# Global Variables
oled = None
grahics = None
MESSAGES_LIST = ['-', '-', '-']
INTERNAL_COUNTER = 0

def init_oled(sda_pin=22, scl_pin=23, width=128, height=64):
    
    global oled, graphics
    
    try:
        i2c_oled = SoftI2C(sda=Pin(sda_pin), scl=Pin(scl_pin))
        oled = ssd1306.SSD1306_I2C(width, height, i2c_oled)
        graphics = gfx.GFX(width, height, oled.pixel)
        return True
    except OSError:
        return False
        
def status(mode):
    
    oled.fill(0)
    
    if mode == 'boot':
        oled.text("[Connecting]",16,4)
        oled.text("  > Hardware",4,20)
        oled.text("    Wifi",4,32)
        oled.text("    UDP",4,44)
    elif mode == 'boot-err':
        oled.text("[Failed]",32,4)
        oled.text("  X Hardware",4,20)
    elif mode == 'wifi':
        oled.text("[Connecting]",16,4)
        oled.text(" OK Hardware",4,20)
        oled.text("  > Wifi",4,32)
        oled.text("    UDP",4,44)
    elif mode == 'wifi-err':
        oled.text("[Failed]",32,4)
        oled.text("  X Wifi",4,32)
    elif mode == 'udp':
        oled.text("[Connecting]",16,4)
        oled.text(" OK Hardware",4,20)
        oled.text(f" OK Wifi",4,32)
        oled.text("  > UDP",4,44)
    elif mode == 'udp-err':
        oled.text("[Failed]",32,4)
        oled.text("  X UDP",4,44)
    else:
        oled.text("[Successful]",16,4)
        oled.text(" OK Hardware",4,20)
        oled.text(f" OK Wifi",4,32)
        oled.text(" OK UDP",4,44)
        
    oled.show()
    
def calibrate():
    
    oled.fill(0)
    oled.text("[Calibration]",14,8)
    oled.text("Tap Button",24,26)
    oled.text("To Set Centre",12,38)
    oled.show()
    
def main_ui(F, B, L ,R , FB_MAG, LR_MAG, PACKETS_SENT, PACKETS_RECV, PULSE, CURRENT_MODE, BARCODE_READ, BARCODE_COUNT):
    
    global INTERNAL_COUNTER
    
    # Get blank canvas
    oled.fill(0)
    
    # Direction UI
    graphics.rect(0,0,64,64,1) # Rectangular Box
    
    if CURRENT_MODE == 0:
        MODE_TEXT = 'Pico'
        
        graphics.line(32,0,32,64,1) # Vertical Line
        graphics.line(0,32,63,32,1) # Horizontal Line
    
        x, y = 32,32 # Start market at centre
        
        if not L == R == F == B == 0: # if not centre
            
            if L: # Move marker left
                x = 32-(((L+1)+(LR_MAG))*7)
                
            if R: # Move marker right
                x = 32+(((R+1)+(LR_MAG))*7)
            
            if F: # Move marker up
                y = 32-(((F+1)+(FB_MAG))*7)
                
            if B: # Move marker down
                y = 32+(((B+1)+(FB_MAG))*7)
                
        graphics.fill_circle(x, y, 6, 1)
        
    elif CURRENT_MODE == 1:
        MODE_TEXT = 'Auto'
        
        graphics.line(0,0,63,63,1) # \
        graphics.line(0,63,63,0,1) # /
        
    elif CURRENT_MODE == 2:
        MODE_TEXT = 'STA1'
        
        graphics.line(0,0,63,63,1) # \
        graphics.line(0,63,63,0,1) # /
    
    # Process Barcode Text:
        
    # If there are no new messages, print back whatever is in memory
    if INTERNAL_COUNTER != BARCODE_COUNT:
        
        INTERNAL_COUNTER += 1
        
        #Format Messgae
        string = str(BARCODE_COUNT) + " " + BARCODE_READ
        
        #Shift everything down 1 slot
        MESSAGES_LIST[2] = MESSAGES_LIST[1]
        MESSAGES_LIST[1] = MESSAGES_LIST[0]
        MESSAGES_LIST[0] = string
    
    # Show messages list    
    oled.text(f"{MESSAGES_LIST[0]}", 68, 36)
    oled.text(f"{MESSAGES_LIST[1]}", 68, 46)
    oled.text(f"{MESSAGES_LIST[2]}", 68, 56)
        
    # Packet Info
    oled.text(f"S:{PACKETS_SENT}", 68, 2)
    oled.text(f"R:{PACKETS_RECV}", 68, 12)
    oled.text(f"{MODE_TEXT}", 68, 22)
    graphics.line(68, 32, 124, 32, 1)
    
    if PULSE:
        graphics.fill_circle(118, 25, 2, 1)
        
    oled.show()
    
def connection_ui(PW_1, PW_2, mode, countdown):
    
    oled.fill(0)
    
    if mode:
        graphics.rect(0,0,128,24,1)
    else:
        graphics.rect(0,25,128,25,1)
        
    oled.text("Hotspot", 3, 4)
    oled.text(f"{PW_1}", 3, 14)
    oled.text("Direct", 3, 28)
    oled.text(f"{PW_2}", 3, 38)
    oled.text(f"{countdown}...", 3, 54)
    oled.show()
    