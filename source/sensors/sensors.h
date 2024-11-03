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
void ultrasonic_callback(uint gpio, uint32_t events);

// infrared.c
#define LEFT_IR_SENSOR 17
#define RIGHT_IR_SENSOR 16
#define BARCODE_IR_SENSOR 6 //18 Is Buzzer
void ir_callback_L(uint gpio, uint32_t events);
void ir_callback_R(uint gpio, uint32_t events);
void ir_callback_BAR(uint gpio, uint32_t events);