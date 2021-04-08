
#include <Arduino.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "config.h"
#include "main.h"

void setup() {
    config_load_settings();

#if PSUC_DEBUG
    Serial.begin(115200);
#endif

    // turn off psu at startup, not very good idea if we'll need to implement watchdog
    digitalWrite(PSUC_ATX_PSU_ON_PIN, PSUC_PSON_SIGNAL ^ HIGH);
    // we set pin to HIGH before mode change to prevent short-time PS-ON peak
    pinMode(PSUC_ATX_PSU_ON_PIN, OUTPUT);

    // configure power button pin mode to detect LOW signal
    pinMode(PSUC_POWER_BUTTON_PIN, INPUT_PULLUP);

    #if PSUC_USE_CONFIRMATION_GPIO
        // configure shut down pin to detect HIGH signal
        pinMode(PSUC_PSU_SHURDOWN_PIN, INPUT);

        // configure gpio request pin
        pinMode(PSUC_GPIO_REQUEST_PIN, OUTPUT);
        digitalWrite(PSUC_GPIO_REQUEST_PIN, LOW);
    #endif

    // configure PSUC leds
    #if PSUC_USE_ONBOARD_LED
        pinMode(PSUC_ARDUINO_LED_PIN, OUTPUT);
        digitalWrite(PSUC_ARDUINO_LED_PIN, LOW);
    #endif

    #if PSUC_USE_EXTERNAL_LED
        pinMode(PSUC_EXTERNAL_LED_PIN, OUTPUT);
        digitalWrite(PSUC_EXTERNAL_LED_PIN, LOW);
    #endif

    #if PSUC_CONTROL_FAN_SPEED
        pinMode(PSUC_CONTROL_FAN_PIN, OUTPUT);
        // set timer 2 divisor to 8 for (Fast) PWM frequency of 31372.55 Hz
        TCCR2B = TCCR2B & B11111000 | B00000001;
    #endif

    // turn off some parts of cpu to lower power consumption
    power_adc_disable();
    power_spi_disable();
    power_twi_disable();

    // set sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    // enable sleep bit
    sleep_enable();

    #if PSUC_USE_CONFIRMATION_GPIO
        gpio_request_sent = 0;
    #endif
}

void atx_poweroff() {
    // turn off PSU module
    digitalWrite(PSUC_ATX_PSU_ON_PIN, PSUC_PSON_SIGNAL ^ HIGH);
#if PSUC_DEBUG
    Serial.println("atx psu power off");
#endif
}

void atx_poweron() {
    // turn on PSU module
    digitalWrite(PSUC_ATX_PSU_ON_PIN, PSUC_PSON_SIGNAL);
#if PSUC_DEBUG
    Serial.println("atx psu power on");
#endif
}

#if PSUC_CONTROL_FAN_SPEED
void case_fan_speed(int duty_cycle) {
    analogWrite(PSUC_CONTROL_FAN_PIN, ((long)255 * duty_cycle) / 100);
}
#endif


void loop() {
    delay(10);
}