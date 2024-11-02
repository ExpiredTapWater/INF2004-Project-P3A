#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "motor.h"
#include "commands.h"

// Set 1 to print PID values
#if 0
#define DEBUG_PRINT(fmt, args...) printf(fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...) // Nothing happens if DEBUG is 0
#endif

// Motor speed target
float target_speed_motor1 = 0;  // Target speed percentage (0-100)
float target_speed_motor2 = 0;  // Target speed percentage (0-100)

bool clockwise_motor1 = true;  // Motor 1 direction
bool clockwise_motor2 = true;  // Motor 2 direction

// PID parameters for Motor 1
float Kp_motor1 = 1.05;
float Ki_motor1 = 0.125;
float Kd_motor1 = 0.05;

// PID parameters for Motor 2
float Kp_motor2 = 1;
float Ki_motor2 = 0.1;
float Kd_motor2 = 0.05;

/*Previous Good with 50ms and 0.05f
// PID parameters for Motor 1
float Kp_motor1 = 2.15;
float Ki_motor1 = 0.25;
float Kd_motor1 = 0.1;

// PID parameters for Motor 2
float Kp_motor2 = 2;
float Ki_motor2 = 0.2;
float Kd_motor2 = 0.1;*/

bool APPLY_PID = false;

// Best: 1.2,0.5,0.2

// Constants
const float wheel_circumference = 0.3318; // in meters (example: 21 cm circumference)
const int pulses_per_revolution = 20;   // Adjust based on your encoder's PPR

const float max_speed_motor1 = 0.5;
const float max_speed_motor2 = 0.5;

// Integral and previous error terms for Motor 1
float integral_motor1 = 0;
float previous_error_motor1 = 0;

// Integral and previous error terms for Motor 2
float integral_motor2 = 0;
float previous_error_motor2 = 0;

// Maximum and minimum integral windup limits
float integral_max = 100;
float integral_min = -100;

void reset_PID() {

    integral_motor1 = 0;
    previous_error_motor1 = 0;
    
    integral_motor2 = 0;
    previous_error_motor2 = 0;

    reset_encoder();
}

float compute_actual_speed(uint32_t pulse_width) {
    if (pulse_width == 0) {
        return 0; // Avoid division by zero
    }

    // Convert pulse width from microseconds to seconds
    float pulse_interval_seconds = pulse_width / 1e6;

    // Calculate the time for one full revolution
    float time_per_revolution = pulse_interval_seconds * pulses_per_revolution;

    // Calculate speed in meters per second
    float speed = wheel_circumference / time_per_revolution;

    return speed;
}

float percent_to_speed(int percent) {
    return (percent / 100.0f) * max_speed_motor1;
}

// Update motor direction and speed (Motor 1 and Motor 2 independently)
void update_motor(float speed_motor1, float speed_motor2, bool clockwise_motor1, bool clockwise_motor2) {

    // Bound and adjust the speed for Motor 1
    if (speed_motor1 > 100) speed_motor1 = 100;
    if (speed_motor1 > 0 && speed_motor1 < 50) speed_motor1 = 50;
    if (speed_motor1 < 0) speed_motor1 = 0;

    // Set PWM speed for Motor 1
    uint16_t pwm_value_motor1 = (uint16_t)((speed_motor1 / 100.0) * 65535);
    pwm_set_gpio_level(MOTOR1_PWM_PIN, pwm_value_motor1);

    // Set direction for Motor 1
    if (clockwise_motor1) {
        gpio_put(MOTOR1_DIR_PIN1, 1);  // Forward
        gpio_put(MOTOR1_DIR_PIN2, 0);  // Reverse
    } else {
        gpio_put(MOTOR1_DIR_PIN1, 0);
        gpio_put(MOTOR1_DIR_PIN2, 1);
    }

    // Bound the speed for Motor 2 (0-100%)
    if (speed_motor2 > 100) speed_motor2 = 100;
    if (speed_motor2 > 0 && speed_motor2 < 50) speed_motor2 = 50;
    if (speed_motor2 < 0) speed_motor2 = 0;

    // Set PWM speed for Motor 2
    uint16_t pwm_value_motor2 = (uint16_t)((speed_motor2 / 100.0) * 65535);
    pwm_set_gpio_level(MOTOR2_PWM_PIN, pwm_value_motor2);

    // Set direction for Motor 2
    if (clockwise_motor2) {
        gpio_put(MOTOR2_DIR_PIN1, 1);  // Forward
        gpio_put(MOTOR2_DIR_PIN2, 0);  // Reverse
    } else {
        gpio_put(MOTOR2_DIR_PIN1, 0);
        gpio_put(MOTOR2_DIR_PIN2, 1);
    }
}

void motor_task(void *params) {
    const float dt = 0.025f;  // Time interval in seconds (50 ms)

    while (1) {
        if (APPLY_PID) {

            // 1. Compute actual speeds (m/s)
            float actual_speed_motor1 = compute_actual_speed(pulse_width_L);
            float actual_speed_motor2 = compute_actual_speed(pulse_width_R);

            // 2. Define target speeds (m/s)
            float target_speed_motor1_ms = percent_to_speed(target_speed_motor1); // Ensure this is set correctly
            float target_speed_motor2_ms = percent_to_speed(target_speed_motor2);

            // 3. Compute errors in speed (m/s)
            float error_motor1 = target_speed_motor1_ms - actual_speed_motor1;
            float error_motor2 = target_speed_motor2_ms - actual_speed_motor2;

            // 4. Update integrals with anti-windup (units: m)
            integral_motor1 += error_motor1 * dt;
            integral_motor1 = fmaxf(fminf(integral_motor1, integral_max), integral_min);

            integral_motor2 += error_motor2 * dt;
            integral_motor2 = fmaxf(fminf(integral_motor2, integral_max), integral_min);

            // 5. Compute derivatives (units: m/s²)
            float derivative_motor1 = (error_motor1 - previous_error_motor1) / dt;
            float derivative_motor2 = (error_motor2 - previous_error_motor2) / dt;

            // 6. Compute control outputs (units: m/s)
            float control_output_motor1 = Kp_motor1 * error_motor1
                                        + Ki_motor1 * integral_motor1
                                        + Kd_motor1 * derivative_motor1;

            float control_output_motor2 = Kp_motor2 * error_motor2
                                        + Ki_motor2 * integral_motor2
                                        + Kd_motor2 * derivative_motor2;

            // 7. Update previous errors
            previous_error_motor1 = error_motor1;
            previous_error_motor2 = error_motor2;

            // 10. Convert control outputs to percentages
            float control_output_percentage_motor1 = (control_output_motor1 / max_speed_motor1) * 100.0f;
            float control_output_percentage_motor2 = (control_output_motor2 / max_speed_motor2) * 100.0f;

            // 11. Bound control outputs to 0-100%
            control_output_percentage_motor1 = fmaxf(fminf(control_output_percentage_motor1, 100.0f), 0.0f);
            control_output_percentage_motor2 = fmaxf(fminf(control_output_percentage_motor2, 100.0f), 0.0f);

            // 12. Update motors with PID outputs
            update_motor(control_output_percentage_motor1, control_output_percentage_motor2, clockwise_motor1, clockwise_motor2);

            // Optional: Print target and actual speeds
            DEBUG_PRINT("[M1] %.2f-%.2f-%.2f [M2] %.2f-%.2f-%.2f\n",
                target_speed_motor1_ms, actual_speed_motor1, control_output_percentage_motor1,
                target_speed_motor2_ms, actual_speed_motor2, control_output_percentage_motor2);
        } else {
            // Stop the motors
            update_motor(target_speed_motor1, target_speed_motor2, clockwise_motor1, clockwise_motor2);
        }

        // 11. Delay to maintain consistent loop timing
        vTaskDelay(pdMS_TO_TICKS(25));
    }
}

// Setup GPIO for both motors
void setup_gpio_motor() {
    // Motor 1 direction pins
    gpio_init(MOTOR1_DIR_PIN1);
    gpio_set_dir(MOTOR1_DIR_PIN1, GPIO_OUT);
    gpio_init(MOTOR1_DIR_PIN2);
    gpio_set_dir(MOTOR1_DIR_PIN2, GPIO_OUT);

    // Motor 2 direction pins
    gpio_init(MOTOR2_DIR_PIN1);
    gpio_set_dir(MOTOR2_DIR_PIN1, GPIO_OUT);
    gpio_init(MOTOR2_DIR_PIN2);
    gpio_set_dir(MOTOR2_DIR_PIN2, GPIO_OUT);
}

// Setup PWM for controlling both motors with independent speeds
void setup_pwm_motor() {

    uint32_t divider = CLOCK_FREQUENCY / (FREQUENCY * 65536);
    
    // PWM setup for Motor 1
    gpio_set_function(MOTOR1_PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num1 = pwm_gpio_to_slice_num(MOTOR1_PWM_PIN);
    pwm_set_clkdiv(slice_num1, divider);
    pwm_set_wrap(slice_num1, 65535);
    pwm_set_enabled(slice_num1, true);

    // PWM setup for Motor 2
    gpio_set_function(MOTOR2_PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num2 = pwm_gpio_to_slice_num(MOTOR2_PWM_PIN);
    pwm_set_clkdiv(slice_num2, divider);
    pwm_set_wrap(slice_num2, 65535);
    pwm_set_enabled(slice_num2, true);
}

void process_motor_commands(void *params) {
    int motor_command = 0;

    while (1) {

        // Wait to receive a command from the queue, storing it in motor_command
        if (xQueueReceive(commands_queue, &motor_command, portMAX_DELAY) == pdPASS) {
            switch (motor_command) {

                case 0: // Stop
                    target_speed_motor1 = 0;
                    target_speed_motor2 = 0;
                    APPLY_PID = false;
                    reset_encoder();
                    reset_PID();
                    break;

// --------------------- FORWARD  --------------------- 

                case CMD_FORWARD_SLOW: // w/PID
                    reset_encoder();
                    clockwise_motor1 = true;
                    clockwise_motor2 = true;                     
                    target_speed_motor1 = 95;
                    target_speed_motor2 = 95;
                    APPLY_PID = true;
                    break;

                case CMD_FORWARD_FAST:
                    clockwise_motor1 = true;
                    clockwise_motor2 = true;                     
                    target_speed_motor1 = 100;
                    target_speed_motor2 = 100;
                    APPLY_PID = false;
                    break;

// --------------------- REVERSE  ---------------------                     

                case CMD_REVERSE_SLOW: // w/PID
                    reset_encoder();
                    clockwise_motor1 = false; 
                    clockwise_motor2 = false; 
                    target_speed_motor1 = 90;
                    target_speed_motor2 = 90;
                    APPLY_PID = true;
                    break;

                case CMD_REVERSE_FAST:
                    clockwise_motor1 = false;
                    clockwise_motor2 = false;
                    target_speed_motor1 = 100;
                    target_speed_motor2 = 100;
                    APPLY_PID = false;
                    break;

// --------------------- LEFT  --------------------- 

                case CMD_LEFT_SLOW:
                    clockwise_motor1 = false;
                    clockwise_motor2 = true;
                    target_speed_motor1 = 0;
                    target_speed_motor2 = 70;
                    APPLY_PID = false;
                    reset_PID();
                    break;

                case CMD_LEFT_FAST:
                    clockwise_motor1 = false;
                    clockwise_motor2 = true;
                    target_speed_motor1 = 60;
                    target_speed_motor2 = 70;
                    APPLY_PID = false;
                    reset_PID();
                    break;

// --------------------- RIGHT  --------------------- 

                case CMD_RIGHT_SLOW:
                    clockwise_motor1 = true;
                    clockwise_motor2 = false;
                    target_speed_motor1 = 70;
                    target_speed_motor2 = 0;
                    APPLY_PID = false;
                    reset_PID();
                    break;

                case CMD_RIGHT_FAST:
                    clockwise_motor1 = true;
                    clockwise_motor2 = false;
                    target_speed_motor1 = 70;
                    target_speed_motor2 = 60; // WRONG
                    APPLY_PID = false;
                    reset_PID();
                    break;

// --------------------- DIAGONAL  --------------------- 

                case CMD_DIAG_TOP_LEFT:
                    clockwise_motor1 = true;
                    clockwise_motor2 = true;
                    target_speed_motor1 = 60;
                    target_speed_motor2 = 90;
                    APPLY_PID = false;
                    break;

                case CMD_DIAG_TOP_RIGHT:
                    clockwise_motor1 = true;
                    clockwise_motor2 = true;
                    target_speed_motor1 = 90+3;
                    target_speed_motor2 = 60;
                    APPLY_PID = false;
                    break;

                case CMD_DIAG_BOTTOM_LEFT:
                    clockwise_motor1 = false;
                    clockwise_motor2 = false;
                    target_speed_motor1 = 60;
                    target_speed_motor2 = 90;
                    APPLY_PID = false;
                    break;

                case CMD_DIAG_BOTTOM_RIGHT:
                    clockwise_motor1 = false;
                    clockwise_motor2 = false;
                    target_speed_motor1 = 90;
                    target_speed_motor2 = 60;
                    APPLY_PID = false;
                    break;

                default:
                    target_speed_motor1 = 0;
                    target_speed_motor2 = 0;
                    APPLY_PID = false;
                    reset_PID();
                    break;
            }
        }
    }
}

void encoder_debug_task(void *params){
    vTaskDelay(pdMS_TO_TICKS(3000));

    while(1){
    float actual_speed_motor1 = compute_actual_speed(pulse_width_L);
    float actual_speed_motor2 = compute_actual_speed(pulse_width_R);
    printf("%.2f-%.2f\n", actual_speed_motor1, actual_speed_motor2);
    vTaskDelay(pdMS_TO_TICKS(200));
    }
    
}
