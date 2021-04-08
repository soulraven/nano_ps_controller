#ifndef _PS_CONTROLLER_
#define _PS_CONTROLLER_

/* ------------- BEGIN CONFIGURATION ---------------- */
// set it to true if you want to get debugging output on serial console (115200)
#define PSUC_DEBUG true

// set to 1 if you want to set HIGH gpio signal for external hardware
// before power down and wait for confirmation signal
// if 0 power will be turned off immediately
#define PSUC_USE_CONFIRMATION_GPIO 0

// set to 1 if you want to use arduino on-board led
#define PSUC_USE_ONBOARD_LED true

// set to 1 if you want to use external (case) led
#define PSUC_USE_EXTERNAL_LED true

// set to 1 if you want to control speed of (case) fan
#define PSUC_CONTROL_FAN_SPEED true

// set fan duty cycle (default is 50%)
#define PSUC_CONTROL_FAN_DUTY_CYCLE 50

// IMPORTANT!!! this is depends on your ATMEGA->[ATX PS_ON PIN] hardware implementation
// set it to LOW if you connected cpu directly to PS_ON
// set to HIGH if you used NPN transistor switch
#define PSUC_PSON_SIGNAL HIGH

// power down system if user pressed button longer than XX seconds
#define PSUC_EMERGENCY_POWER_DOWN_DELAY 10

// return to sleep mode if gpio event didn't occur after XX seconds
#define PSUC_RETURN_TO_POWERON_SLEEP_IN 60
/* -------------- END CONFIGURATION ----------------- */

/* -------------- BUTTONS MAPPING ------------------*/

// input (irq): external "power on/off" (momentary) button connected to ground
// D2 Digital Input 2
#define PSUC_POWER_BUTTON_PIN 2

// input irq: external reset (momentary) button connected to ground
// D3 Digital Input 3
#define PSUC_RESET_BUTTON_PIN 3

/* -------------- LEDS MAPPING ------------------*/

// arduino onboard led
#define PSUC_ARDUINO_LED_PIN LED_BUILTIN

// external case led
#define PSUC_EXTERNAL_LED_PIN 6

// case fan, use pin 11 (asynchronous clock that is not stopped with SLEEP_MODE_PWR_SAVE)
// change timer configuration if you want to remap this pin
#define PSUC_CONTROL_FAN_PIN 11

/* -------------- INPUT/OUT PINS ------------------*/

// output: connected to ATX-24 PS_ON (16) pin
// This pin is used to controll the ATX-24 power supply
// D5 Digital Output
#define PSUC_ATX_PSU_ON_PIN 5

// output: connected to external GPIO
// HIGH state is signal for PSU clients to close their business and set HIGH on PSUC_PSU_SHURDOWN_PIN
// for power down
#define PSUC_GPIO_REQUEST_PIN 5

// input (irq): external request for immediate PSU shutdown
// D4 Digital Input 4
#define PSUC_PSU_SHURDOWN_PIN 4

enum t_psuc_state {
    // PSUC is initializing
    PSUC_INIT,
    // application is sleeping and waiting for external interrupt, PSU is OFF
    PSUC_SLEEP_OFF,
    // application is sleeping and waiting for external interrupt, PSU is ON
    PSUC_SLEEP_ON,
    // processing turn PSU OFF
    PSUC_POWER_OFF,
    // power off requested by user
    PSUC_EMERGENCY_POWER_OFF,
    // processing turn PSU ON
    PSUC_POWER_ON,
#if PSUC_USE_CONFIRMATION_GPIO
    // read GPIO signals
    PSUC_WAIT_GPIO_READ,
    // confirmate HIGH signal on GPIO and shutdown PSU
    PSUC_WAIT_GPIO_CONFIRMATION
#endif
};

static volatile t_psuc_state psuc_state;
byte power_btn_is_pressed;
byte poweroff_signal_is_received;
#if PSUC_USE_CONFIRMATION_GPIO
    byte gpio_request_sent;
    unsigned long last_time_wait_alone;
    unsigned long start_time_wait_gpio;
#endif

#if PSUC_CONTROL_FAN_SPEED
    static void case_fan_speed(int duty_cycle);
#endif

#endif  // _PS_CONTROLLER_