/**
 * 
 **/

#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))

struct NanoSettings {
    bool lastPowerState;
    int fanDutyCycle;
};

extern NanoSettings my_setting;

// -------------------------------------------------------------------
// Load saved settings
// -------------------------------------------------------------------
extern void config_load_settings();

// -------------------------------------------------------------------
// Reset the config back to defaults
// -------------------------------------------------------------------
extern void config_reset();

#endif  // _CONFIG_H