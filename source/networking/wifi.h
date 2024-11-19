#include "queue.h" //Fixes issue with importing hardware_pwm conflict

// Networking Stuff
#define WIFI_CHANNEL 11
#define UDP_RECV_PORT 2004

// ---- HOTSPOT ----
#define HOTSPOT_SSID "CY"
#define HOTSPOT_PASSWORD "wifipassword"

// Address of the pico mounted to the car
#define HOTSPOT_IP_STRING "172.20.10.3"
#define HOTSPOT_CAR_IP_1 172
#define HOTSPOT_CAR_IP_2 20
#define HOTSPOT_CAR_IP_3 10
#define HOTSPOT_CAR_IP_4 3

// Address this pico should set itself to
#define HOTSPOT_PICO_IP_1 172
#define HOTSPOT_PICO_IP_2 20
#define HOTSPOT_PICO_IP_3 10
#define HOTSPOT_PICO_IP_4 4

// Address for the gateway device
#define HOTSPOT_GATEWAY_IP_1 172
#define HOTSPOT_GATEWAY_IP_2 20
#define HOTSPOT_GATEWAY_IP_3 10
#define HOTSPOT_GATEWAY_IP_4 1

// ---- DIRECT CONNECTION (AP) ----
#define AP_SSID "PicoW-P3A"
#define AP_PASSWORD "12345678"

// Address of the pico mounted to the car
#define AP_IP_STRING "192.168.4.1"
#define AP_CAR_IP_1 192
#define AP_CAR_IP_2 168
#define AP_CAR_IP_3 4
#define AP_CAR_IP_4 1

// Address this pico should set itself to
#define AP_PICO_IP_1 192
#define AP_PICO_IP_2 168
#define AP_PICO_IP_3 4
#define AP_PICO_IP_4 3

// Enables debug print statements
#define DEBUG 0
#if DEBUG
#define DEBUG_PRINT(fmt, args...) printf(fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...) // Nothing happens if DEBUG is 0
#endif

typedef enum {
    BOOT = 0,
    WIFI,
    MAIN
} program_state_t;

// ---- Queues ----
extern QueueHandle_t received_queue;