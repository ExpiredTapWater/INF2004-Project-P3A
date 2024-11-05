#include "queue.h"
#include "semphr.h"


#define THRESHOLD 30000
#define MIN_THRESHOLD 100

/*
#define NARROW_WIDTH 30000
#define WIDE_WIDTH (NARROW_WIDTH*3)
#define VARIANCE 30
#define NARROW_RANGE_LOWER (NARROW_WIDTH - VARIANCE)
#define NARROW_RANGE_UPPER (NARROW_WIDTH + VARIANCE)
#define WIDE_RANGE_LOWER (WIDE_WIDTH - VARIANCE)
#define WIDE_RANGE_UPPER (WIDE_WIDTH + VARIANCE)
*/

// From main.c
extern QueueHandle_t barcodes_queue;

// From infrared.c
extern SemaphoreHandle_t Barcode_BinarySemaphore;
extern volatile uint32_t IR_pulse_width_BAR;