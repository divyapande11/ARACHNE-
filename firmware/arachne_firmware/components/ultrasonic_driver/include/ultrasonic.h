#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "driver/gpio.h"
#include "esp_err.h"

#define TRIG_PIN 4
#define ECHO_PIN 5

void ultrasonic_init(void); 
void ultrasonic_get_distance_cm(void); 

#endif