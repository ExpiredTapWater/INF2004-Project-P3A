// encoder.c
#define LEFT_ENCODER 0
#define RIGHT_ENCODER 1
void encoder_callback_L(uint gpio, uint32_t events);
void encoder_callback_R(uint gpio, uint32_t events);

// ultrasonic.c
#define TRIG_PIN 4
#define ECHO_PIN 5
#define MIN_DISTANCE 20
#define CALIBRATION_OFFSET 0
#define CALIBRATION_SCALING 1
#define TIMEOUT 100
#define POLLING_DELAY 250
void ultrasonic_callback(uint gpio, uint32_t events);

// infrared.c
#define IR_SENSOR 17
#define BARCODE_IR_SENSOR 16
void ir_callback(uint gpio, uint32_t events);
void ir_callback_BAR(uint gpio, uint32_t events);

// motor.c
void disable_warning();

// Set 1 to print
#if 1
#define DEBUG_PRINT(fmt, args...) printf(fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...) // Nothing happens if DEBUG is 0
#endif