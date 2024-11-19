#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mpu6050.h"
#include <math.h>
 
#define MPU6050_SDA_PIN 26
#define MPU6050_SCL_PIN 27
#define REFRESH_DELAY 50
mpu6050_t mpu6050;
mpu6050_vectorf_t *accel;

float X_NEUTRAL = 0.0f;
float Y_NEUTRAL = 0.0f;
float NEUTRAL_DEADZONE = 1.5f;
float HIGH_THRESHOLD = 5.0f;

// Static should already set these to 0
static float X, Y, X_abs, Y_abs;
static bool F, B, L, R;
static bool FB_MAG, LR_MAG ;
static uint8_t F_MAG, B_MAG, L_MAG, R_MAG;
volatile uint8_t mappings;

void setup_sensor(){

    // MPU6050
    i2c_init(i2c1, 400000);
    gpio_set_function(MPU6050_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MPU6050_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MPU6050_SDA_PIN);
    gpio_pull_up(MPU6050_SCL_PIN);
    
    // Init sensor
    mpu6050 = mpu6050_init(i2c1, MPU6050_ADDRESS_A0_GND);
    mpu6050_begin(&mpu6050);

    // Set range of accelerometer
    mpu6050_set_range(&mpu6050, MPU6050_RANGE_2G);

    // Disable temperature, gyroscope and accelerometer readings
    mpu6050_set_temperature_measuring(&mpu6050, false);
    mpu6050_set_gyroscope_measuring(&mpu6050, false);
    mpu6050_set_accelerometer_measuring(&mpu6050, true);

    // Disable all interrupt flags
    mpu6050_set_int_free_fall(&mpu6050, false);
    mpu6050_set_int_motion(&mpu6050, false);
    mpu6050_set_int_zero_motion(&mpu6050, false);

}

void set_neutral(){

    // Get sensor event
    mpu6050_event(&mpu6050);

    // Get sensor data
    accel = mpu6050_get_accelerometer(&mpu6050);

    // Update neutral baselines
    X_NEUTRAL = accel->x;
    Y_NEUTRAL = accel->y;

    printf("New X: %f New Y: %f\n", X_NEUTRAL, Y_NEUTRAL);

}

void get_binary_mappings(){

    /*
     Directional Information is sent binary formated as 'Forward, Back, Left, Right'
    Then intensity in the same order. 0 being low speed, and 1 being high speed.
    '00000000' is stop
    '10001000' is forward at high speed
    '10100000' is forward-left at low speed   
    */

    // Read sensor values
    mpu6050_event(&mpu6050);
    accel = mpu6050_get_accelerometer(&mpu6050);
    X = accel->x;
    Y = accel->y;

    // Reset all values to zero
    F = 0, B = 0, L = 0, R = 0;
    FB_MAG = 0, LR_MAG = 0 ;
    mappings = 0;

    // ------ Check Left-Right tilt ------
    X_abs = fabsf(X); // Get absolute value

    // Check if over neutral deadzone
    if (X_abs >= NEUTRAL_DEADZONE){

        // Sensor over deadzone threshold, check if magnitude
        if (X_abs >= HIGH_THRESHOLD){
            LR_MAG = 1;
        }else{
            LR_MAG = 0;
        }

        // Check for direction now
        if (X >= X_NEUTRAL){ // +Ve is right, -Ve is left
            R = 1;
        }else{
            L = 1;
        }

    } 

    // ------ Check Forward-Back tilt ------
    Y_abs = fabsf(Y); // Get absolute value

    // Check if over neutral deadzone
    if (Y_abs >= NEUTRAL_DEADZONE){

        // Sensor over deadzone threshold, check if magnitude
        if (Y_abs >= HIGH_THRESHOLD){
            FB_MAG = 1;
        }else{
            FB_MAG = 0;
        }

        // Check for direction now
        if (Y >= Y_NEUTRAL){ // +Ve is forward, -Ve is back
            F = 1;
        }else{
            B = 1;
        }

    }

    // ------ Restrict diagonal movement ------
    /* Explaination: a-d corresponds to the 4 LSB in the mapping
    We want to simplify diagonal movement, so no cases of diagonal
    left with slow or high speeds. We also give one axis say forward-back
    priority over another. In this case, F-B will override any side movement*/

    F_MAG = (F && !(L || R)) ? FB_MAG : 0;
    B_MAG = (B && !(L || R)) ? FB_MAG : 0;
    L_MAG = (L && !(F || B)) ? LR_MAG : 0;
    R_MAG = (R && !(F || B)) ? LR_MAG : 0;

    // ------ Create Mapping ------
    // Set leftmost 4 bits for F, B, L, R
    mappings |= (F << 7); // Forward
    mappings |= (B << 6); // Back
    mappings |= (L << 5); // Left
    mappings |= (R << 4); // Right

    mappings |= (F_MAG << 3); // Forward Magnitude
    mappings |= (B_MAG << 2); // Back Magnitude
    mappings |= (L_MAG << 1); // Left Magnitude
    mappings |= R_MAG;        // Right Magnitude
}

// Debug function to check mappings
void print_binary(uint8_t value) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}

void accelerometer_task(){

    vTaskDelay(pdMS_TO_TICKS(3000));
    printf("Staring Accelerometer Task\n");

    while(1){
        get_binary_mappings();
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY));
    }

}

void sample_accelerometer_task(){

    vTaskDelay(pdMS_TO_TICKS(3000));
    printf("Sample Task\n");
    uint8_t temp = 0;

    while (1){

        /*
        // Fetch all data from the sensor | I2C is only used here
        mpu6050_event(&mpu6050);

        // Pointers to float vectors with all the results
        accel = mpu6050_get_accelerometer(&mpu6050);

        // Print all the measurements
        printf("Accelerometer: %f, %f\n", accel->x, accel->y);
        */

        get_binary_mappings();
        print_binary(mappings);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY));

    }
 
}