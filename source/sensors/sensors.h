// encoder.c
#define LEFT_ENCODER 0
#define RIGHT_ENCODER 1
void encoder_callback(uint gpio, uint32_t events);

// ultrasonic.c
#define TRIG_PIN 4
#define ECHO_PIN 5
#define MIN_DISTANCE 20
#define CALIBRATION_OFFSET 0
#define CALIBRATION_SCALING 1
#define TIMEOUT 100
void ultrasonic_callback(uint gpio, uint32_t events);