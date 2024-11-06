import time
import math
import MPU6050
import uasyncio as asyncio
from machine import Pin, SoftI2C, Timer

# --- Global ---
mpu = None

# --- Default Threshold Values ---
X_NEUTRAL = 0.0   # X is the left(+) right(-) cant
Y_NEUTRAL = 0.0 # Y is the forward-back cant 
Z_NEUTRAL = 0.8 # Z is the rotation (facing sky or ground)
NEUTRAL_DEADZONE = 0.20 # Deadzone where values are counted as neutral

HIGH_THRESHOLD = 0.60

def init_sensor(sda_pin=32, scl_pin=21):
    
    global mpu
    
    try:
        i2c_mpu = SoftI2C(sda=sda_pin, scl=scl_pin)
        mpu = MPU6050.MPU6050(i2c_mpu)
        mpu.wake()
        time.sleep_ms(50)
        mpu.read_accel_data()
        
        return True
    
    except OSError:
        return False
    
def calibrate():
    global X_NEUTRAL, Y_NEUTRAL, Z_NEUTRAL
    X, Y, Z = mpu.read_accel_data()
    X_NEUTRAL = X
    Y_NEUTRAL = Y
    Z_NEUTRAL = Z
    
def truncate(value: float) -> str:
    return f"{value:.2f}"
    
async def get_direction():
    '''
    Directional Information is sent binary formated as 'Forward, Back, Left, Right'
    Then intensity in the same order. 0 being low speed, and 1 being high speed.
    '00000000' is stop
    '10001000' is forward at high speed
    '10100000' is forward-left at low speed
    '''
    
    F = B = L = R = 0
    FB_MAG = LR_MAG = 0
    
    # Read sensor values
    X, Y, Z = mpu.read_accel_data()
    
    # ---- Left-Right Cant ----
    X_ABS = abs(X - X_NEUTRAL)
    
    # Check if over deadzone
    if X_ABS > NEUTRAL_DEADZONE:
            
        # Get Magnitude
        if X_ABS <= HIGH_THRESHOLD:
            LR_MAG = 0 # Low Speed
        else:
            LR_MAG = 1 # High Speed
            
        # Get Direction
        if X >= X_NEUTRAL:
            L = 1
            
        else: # X < X_Neutral
            R = 1
            
    # ---- Forward-Back Cant ----
    Y_ABS = abs(Y - Y_NEUTRAL)
    
    # Check if over deadzone
    if Y_ABS > NEUTRAL_DEADZONE:
            
        # Get Magnitude
        if Y_ABS <= HIGH_THRESHOLD:
            FB_MAG = 0
        else:
            FB_MAG = 1
            
        # Get Direction
        if Y >= Y_NEUTRAL:
            B = 1
        else: # Y < Y_Neutral
            F = 1
        
    return F, B, L, R, FB_MAG, LR_MAG