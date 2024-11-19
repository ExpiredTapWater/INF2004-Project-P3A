// Pico
#include <string.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
// Networking
#include "lwip/udp.h"
#include "lwip/netifapi.h"
// Our Headers
#include "wifi.h"

#define MESSAGE_BUFFER 12

// Variables to keep track of the UDP PCB and client IP addresses
static struct udp_pcb *udp_pcb = NULL;
volatile uint16_t total_packets_sent = 0;
volatile uint16_t total_packets_received = 0;
ip_addr_t car_ip;

// States
volatile bool connection_state = false;
volatile bool handshake_state = false;

// From io_handler.c
extern volatile program_state_t current_state;

// From display.c
extern volatile bool connection_cursor;

// From main.c
extern QueueHandle_t received_queue;

// Function prototype for the UDP receive callback
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                          const ip_addr_t *addr, u16_t port);

void start_udp(void) {
    // Create a new UDP PCB
    udp_pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    if (udp_pcb == NULL) {
        printf("Failed to create UDP PCB\n");
        return;
    }

    // Bind the UDP PCB to the specified port
    if (udp_bind(udp_pcb, IP_ANY_TYPE, UDP_RECV_PORT) != ERR_OK) {
        printf("Failed to bind UDP PCB\n");
        udp_remove(udp_pcb);
        udp_pcb = NULL;
        return;
    }

    // Set the receive callback function
    udp_recv(udp_pcb, udp_receive_callback, NULL);

    printf("UDP initialized and listening on port %d\n", UDP_RECV_PORT);
}

// UDP receive callback function
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                          const ip_addr_t *addr, u16_t port) {
    if (p == NULL) {
        return;
    }

    // Copy the received data into a buffer
    char buffer[MESSAGE_BUFFER];  // Adjust size as needed
    int len = p->tot_len > sizeof(buffer) - 1 ? sizeof(buffer) - 1 : p->tot_len;
    pbuf_copy_partial(p, buffer, len, 0);
    buffer[len] = '\0';  // Null-terminate the string

    // Print the received message
    DEBUG_PRINT("Received UDP packet from %s:%d\n", ipaddr_ntoa(addr), port);

    // Add to queue
    if (xQueueSend(received_queue, &buffer, 0U) != pdPASS) {
        printf("Queue is full!\n");
    }

    // Free the pbuf
    pbuf_free(p);
}

// Function to send UDP packets to a specific client
void send_udp_packet(const char *data, const ip_addr_t *client_ip, uint16_t client_port) {
    if (udp_pcb == NULL) {
        printf("UDP PCB is not initialized\n");
        return;
    }

    if (client_ip->addr == IPADDR_ANY || client_port == 0) {
        printf("Invalid client IP or port\n");
        return;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(data), PBUF_RAM);
    if (p == NULL) {
        printf("Failed to allocate pbuf for UDP\n");
        return;
    }

    memcpy(p->payload, data, strlen(data));
    err_t err = udp_sendto(udp_pcb, p, client_ip, client_port);
    if (err != ERR_OK) {
        DEBUG_PRINT("Failed to send UDP packet: %d\n", err);
    } else {
        DEBUG_PRINT("UDP packet sent to %s:%d\n", ipaddr_ntoa(client_ip), client_port);
    }
    pbuf_free(p);
}

// Function to send a single-byte UDP packet to a specific client
void send_udp_byte_packet(uint8_t data, const ip_addr_t *client_ip, uint16_t client_port) {

    // Allocate a pbuf for a single byte
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 1, PBUF_RAM);
    if (p == NULL) {
        printf("Failed to allocate pbuf for UDP\n");
        return;
    }

    // Set the payload to the single byte
    *((uint8_t *)p->payload) = data;

    // Send the packet
    err_t err = udp_sendto(udp_pcb, p, client_ip, client_port);
    if (err != ERR_OK) {
        DEBUG_PRINT("Failed to send UDP packet: %d\n", err);
    } else {
        DEBUG_PRINT("UDP byte packet sent to %s:%d\n", ipaddr_ntoa(client_ip), client_port);
    }

    // Free the allocated pbuf
    pbuf_free(p);
}

void handshake(){

    char message[MESSAGE_BUFFER] = {0};

    // Send 11111111 handshake packet
    send_udp_byte_packet(0xFF, &car_ip, 2004);

    while(1){

        // Wait for message
        xQueueReceive(received_queue, &message, portMAX_DELAY);
        if (strcmp(message, "Pico-Hello") == 0){
            handshake_state = true;
            printf("Handshake Done\n");
            break;
        }
    }
}

void wifi_task(void *pvParameters) {

    // Manually assign IP
    struct ip4_addr static_ip, gateway, netmask;

    // Wait for 3 seconds for setup
    vTaskDelay(pdMS_TO_TICKS(3000));

    printf("Waiting for wifi start command\n");

    // Wait till io_handler notifies to start wifi setup process
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Enable Station mode
    cyw43_arch_enable_sta_mode();

    // Start wifi connection based on user's chosen connection method
    if(connection_cursor){ // Hotspot Mode Selected

        printf("Connecting to hotspot\n");

        // Manually assign IP
        IP4_ADDR(&static_ip, HOTSPOT_PICO_IP_1, HOTSPOT_PICO_IP_2, HOTSPOT_PICO_IP_3, HOTSPOT_PICO_IP_4);
        IP4_ADDR(&gateway, HOTSPOT_GATEWAY_IP_1, HOTSPOT_GATEWAY_IP_2, HOTSPOT_GATEWAY_IP_3, HOTSPOT_GATEWAY_IP_4);
        IP4_ADDR(&netmask, 255, 255, 255, 0);

        // Set IP address, gateway, and netmask for STA mode
        netif_set_addr(&cyw43_state.netif[CYW43_ITF_STA], &static_ip, &netmask, &gateway);

        if (cyw43_arch_wifi_connect_timeout_ms(HOTSPOT_SSID, HOTSPOT_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
            printf("Failed to connect to Hotspot\n");
            return;
        }

        // Wait for IP address assignment
        const ip4_addr_t *ip = &cyw43_state.netif[0].ip_addr;
        for (int i = 0; i < 10; i++) {
            if (!ip4_addr_isany_val(*ip)) {
                break;
            }
            cyw43_arch_poll();
            sleep_ms(500);
        }

        // Check if IP address is assigned
        if (!ip4_addr_isany_val(*ip)) {
            printf("IP Address assigned: %s\n", ip4addr_ntoa(ip));
        } else {
            printf("Failed to obtain IP address.\n");
        }

        // Set the car IP for Hotspot mode
        IP4_ADDR(&car_ip, HOTSPOT_CAR_IP_1, HOTSPOT_CAR_IP_2, HOTSPOT_CAR_IP_3, HOTSPOT_CAR_IP_4);

    }else{ // Direct (Access Point Mode)

        printf("Connecting to access point\n");

        // Manually assign IP
        IP4_ADDR(&static_ip, AP_PICO_IP_1, AP_PICO_IP_2, AP_PICO_IP_3, AP_PICO_IP_4);
        IP4_ADDR(&gateway, AP_CAR_IP_1, AP_CAR_IP_2, AP_CAR_IP_3, AP_CAR_IP_4);
        IP4_ADDR(&netmask, 255, 255, 255, 0);

        netif_set_addr(&cyw43_state.netif[CYW43_ITF_STA], &static_ip, &netmask, &gateway);

        if (cyw43_arch_wifi_connect_timeout_ms(AP_SSID, AP_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
            printf("Failed to connect to AP\n");
            return;
        }

        // Set the car IP for AP mode
        IP4_ADDR(&car_ip, AP_CAR_IP_1, AP_CAR_IP_2, AP_CAR_IP_3, AP_CAR_IP_4);
    }

    printf("Connected to Wi-Fi network\n");

    // Initialize UDP
    start_udp();

    connection_state = true;

    // Peform Handshake
    handshake();
   
    // Main loop
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}