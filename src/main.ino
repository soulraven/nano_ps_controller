
#include <Arduino.h>
#include <EasyButton.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "config.h"
#include "main.h"

EasyButton powerBtn(PSUC_POWER_BUTTON_PIN, BUTTONS_DEBOUNCE_TIME, PSUC_POWER_BUTTON_PIN_PULLUP);
EasyButton resetBtn(PSUC_RESET_BUTTON_PIN, BUTTONS_DEBOUNCE_TIME, PSUC_RESET_BUTTON_PIN_PULLUP);

void SerialBegin() {
#if PSUC_DEBUG
    Serial.begin(115200);
    while (!Serial) {
        delay(20);
    }
#endif
}

void SerialEnd() {
#if PSUC_DEBUG
    Serial.println("Entering sleep mode. Disable Serial");
    Serial.end();
#endif
}

void process_sleep() {
    // enable sleep bit
    sleep_enable();

    attachInterrupt(digitalPinToInterrupt(PSUC_POWER_BUTTON_PIN), power_button_rq, LOW);
    attachInterrupt(digitalPinToInterrupt(PSUC_RESET_BUTTON_PIN), power_button_rq, LOW);
    // Disable Serial
    SerialEnd();
    // going to sleep
    sleep_mode();
    // first thing after waking from sleep
    sleep_disable();
    // disable interrupts
    detachInterrupt(digitalPinToInterrupt(PSUC_POWER_BUTTON_PIN));
    detachInterrupt(digitalPinToInterrupt(PSUC_RESET_BUTTON_PIN));
    // Enable Serial
    SerialBegin();
}

void power_button_rq() {
    // button is pressed
}

void power_down_rq() {
    // external request for immediate psu down is received
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

void led_control(byte state) {
    if (state) {
#if PSUC_USE_EXTERNAL_LED
        digitalWrite(PSUC_EXTERNAL_LED_PIN, HIGH);
#endif
#if PSUC_USE_ONBOARD_LED
        digitalWrite(PSUC_ARDUINO_LED_PIN, HIGH);
#endif
    } else {
#if PSUC_USE_EXTERNAL_LED
        digitalWrite(PSUC_EXTERNAL_LED_PIN, LOW);
#endif
#if PSUC_USE_ONBOARD_LED
        digitalWrite(PSUC_ARDUINO_LED_PIN, LOW);
#endif
    }
}

void led_state_control(t_psuc_state state) {
    switch (state) {
        case PSUC_RESET:
            led_control(LOW);
            break;
        case PSUC_SLEEP_OFF:
            led_control(LOW);
            break;
        case PSUC_SLEEP_ON:
            
            break;
        case PSUC_POWER_OFF:
            break;
        case PSUC_POWER_ON:
            led_control(HIGH);
            break;
        default:
            break;
    }
}

void change_state(t_psuc_state new_state) {
    switch (new_state) {
        case PSUC_SLEEP_OFF:
            // reset current input signals states
            reset_signals();
#if PSUC_DEBUG
            Serial.println("PSUC_SLEEP_OFF");
#endif
            break;
        case PSUC_SLEEP_ON:
            // reset current input signals states
            reset_signals();
#if PSUC_DEBUG
            Serial.println("PSUC_SLEEP_ON");
#endif
            break;
        case PSUC_RESET:
            Serial.println("PSUC_RESET");
            break;
        case PSUC_EMERGENCY_POWER_OFF:
#if PSUC_DEBUG
            Serial.println("PSUC_EMERGENCY_POWER_OFF");
#endif
            break;
        case PSUC_POWER_OFF:
#if PSUC_DEBUG
            Serial.println("PSUC_POWER_OFF");
#endif
            break;
        case PSUC_POWER_ON:
#if PSUC_DEBUG
            Serial.println("PSUC_POWER_ON");
#endif
            break;
        default:
            break;
    }
    psuc_state = new_state;
}

void psu_control(t_psuc_state state) {
    switch (state) {
        case PSUC_SLEEP_OFF:
            // process_sleep();
            // if (power_btn_is_pressed) {
            //     change_state(PSUC_POWER_ON);
            // }
            break;
        case PSUC_SLEEP_ON:
            // process_sleep();
            // if (power_btn_is_pressed) {
            //     change_state(PSUC_POWER_OFF);
            // }
            break;
        case PSUC_RESET:
            change_state(PSUC_POWER_OFF);
            led_control(LOW);
            delay(2000);
            change_state(PSUC_POWER_ON);
            led_control(HIGH);
            break;
        case PSUC_POWER_OFF:
            atx_poweroff();
            delay(1000);
#if PSUC_CONTROL_FAN_SPEED
            case_fan_speed(0);
#endif
            change_state(PSUC_SLEEP_OFF);
            break;
        case PSUC_EMERGENCY_POWER_OFF:
            change_state(PSUC_POWER_OFF);
            break;
        case PSUC_POWER_ON:
            atx_poweron();
            delay(1000);
#if PSUC_CONTROL_FAN_SPEED
            case_fan_speed(PSUC_CONTROL_FAN_DUTY_CYCLE);
#endif
            change_state(PSUC_SLEEP_ON);
            break;
        default:
            break;
            }
}

void powerBtnISR() {
    powerBtn.read();
}

void resetBtnISR() {
    resetBtn.read();
}
 
void powerBtnLongPress() {
    Serial.print("Long press detected, try Emergency Power Off");
    change_state(PSUC_EMERGENCY_POWER_OFF);
}

void powerBtnShortPress() {
    if (psuc_state != PSUC_SLEEP_ON) {
        change_state(PSUC_POWER_ON);
        power_btn_is_pressed = true;
    }
}

void resetBtnShortPress() {
    if (psuc_state == PSUC_POWER_ON || psuc_state == PSUC_SLEEP_ON && (psuc_state != PSUC_SLEEP_OFF)) {
        change_state(PSUC_RESET);
        reset_btn_is_pressed = true;
    }
}

void reset_signals() {
    power_btn_is_pressed = false;
    reset_btn_is_pressed = false;
}

void setup() {
    config_load_settings();

    SerialBegin();
    Serial.println("Starting up...");

    // Initialize power button
    powerBtn.begin();
    // Add the callback function to be called when the button is pressed for at least the given time.
    powerBtn.onPressedFor(PSUC_EMERGENCY_POWER_DOWN_DELAY, powerBtnLongPress);
    powerBtn.onPressed(powerBtnShortPress);

    if (powerBtn.supportsInterrupt()) {
        powerBtn.enableInterrupt(powerBtnISR);
        Serial.println("Power button use ISR");
    }

    // Initialize reset button
    resetBtn.begin();
    resetBtn.onPressed(resetBtnShortPress);

    if (resetBtn.supportsInterrupt()) {
        resetBtn.enableInterrupt(resetBtnISR);
        Serial.println("Reset button use ISR");
    }

    // we set pin to HIGH before mode change to prevent short-time PS-ON peak
    pinMode(PSUC_ATX_PSU_ON_PIN, OUTPUT);

    // attach the power button input PIN
    // turn off psu at startup, not very good idea if we'll need to implement watchdog
    digitalWrite(PSUC_ATX_PSU_ON_PIN, PSUC_PSON_SIGNAL ^ HIGH);

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
        TCCR2B = TCCR2B & (B11111000 | B00000001);
    #endif

    // turn off some parts of cpu to lower power consumption
    power_adc_disable();
    power_spi_disable();
    power_twi_disable();

    // enable sleep bit
    sleep_enable();
    // set sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);

    #if PSUC_USE_CONFIRMATION_GPIO
        gpio_request_sent = 0;
    #endif

    reset_signals();

    psuc_state = PSUC_INIT;
}

void loop() {    

    if (powerBtn.supportsInterrupt()) {
        powerBtn.update();
    } else {
        powerBtn.read();
    }

    if (resetBtn.supportsInterrupt()) {
        resetBtn.update();
    } else {
        resetBtn.read();
    }

    led_state_control(psuc_state);
    psu_control(psuc_state);
    delay(10);
}