#include <stdio.h>          // for printf() - we're printing distance to serial
#include "esp_timer.h"      // for esp_timer_get_time() - precise microsecond timestamps
#include "esp_rom_sys.h"    // for esp_rom_delay_us() - the delayMicroseconds() equivalent
#include "driver/gpio.h"    // for gpio_set_level(), gpio_get_level(), pin config
#include "ultrasonic.h"     // our own header - trig/echo pin definitions, function declarations
#define TRIG_TIMEOUT_US 30000   // if echo never responds, give up after 30ms (prevents infinite hang)

void ultrasonic_init(void) {
    gpio_config_t trig_conf = {
        .pin_bit_mask = (1ULL << TRIG_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&trig_conf);

    gpio_config_t echo_conf = {
        .pin_bit_mask = (1ULL << ECHO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&echo_conf);
}

void ultrasonic_get_distance_cm(void) {

    // Step 1: Clear trigger pin, force known LOW state
    gpio_set_level(TRIG_PIN, 0);
    esp_rom_delay_us(2);

    // Step 2: Send 10us HIGH pulse (per HC-SR04 datasheet requirement)
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);

    // Step 3: Wait for echo pin to go HIGH (sensor starting its response)
    int64_t wait_start = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 0) {
        if ((esp_timer_get_time() - wait_start) > TRIG_TIMEOUT_US) {
            printf("Timeout waiting for echo to start\n");
            return;
        }
    }

    // Step 4: Echo just went HIGH - start the real measurement clock
    int64_t echo_start = esp_timer_get_time();

    // Step 5: Wait for echo pin to go back LOW (sensor done responding)
    while (gpio_get_level(ECHO_PIN) == 1) {
        if ((esp_timer_get_time() - echo_start) > TRIG_TIMEOUT_US) {
            printf("Timeout waiting for echo to end\n");
            return;
        }
    }
    int64_t echo_end = esp_timer_get_time();

    // Step 6: Calculate pulse duration, then distance
    int64_t pulse_duration_us = echo_end - echo_start;
    float distance_cm = (34300.0f * pulse_duration_us / 1000000.0f) / 2.0f;

    // Step 7: Threshold check + print
    if (distance_cm <= 20.0f) {
        printf("Obstacle detected! Distance: %.2f cm - STOP, tilt right\n", distance_cm);
    } else {
        printf("Distance: %.2f cm - keep walking\n", distance_cm);
    }
}