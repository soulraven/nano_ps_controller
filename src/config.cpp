/**
 * 
 **/

#include "config.h"

#include <Arduino.h>
#include <EEPROM.h>  // Save config settings

NanoSettings my_setting;

#define EEPROM_SIZE                 512
#define EEPROM_POWER_STATE_SIZE     1
#define EEPROM_FAN_DUTY_CYCLE_SIZE  2

#define EEPROM_BEGIN                0
#define EEPROM_POWER_STATE_END      (EEPROM_BEGIN + EEPROM_POWER_STATE_SIZE)

#define EEPROM_FAN_DUTY_CYCLE_BEGIN EEPROM_POWER_STATE_END
#define EEPROM_FAN_DUTY_CYCLE_END   (EEPROM_FAN_DUTY_CYCLE_BEGIN + EEPROM_FAN_DUTY_CYCLE_SIZE)

// -------------------------------------------------------------------
// Reset EEPROM, wipes all settings
// -------------------------------------------------------------------
void ResetEEPROM() {
    for (int i = 0; i < EEPROM_SIZE; ++ i) {
        //skip already "empty" addresses
        if (EEPROM.read(i) != 0) {
            EEPROM.write(i, 0);  //write 0 to address i
        }
    }
}

void EEPROM_read_string(int start, int count, String &val) {
    for (int i = 0; i < count; ++i) {
        byte c = EEPROM.read(start + i);
        if (c != 0 && c != 255)
            val += (char)c;
    }
}

void EEPROM_write_string(int start, int count, String val) {
    for (int i = 0; i < count; ++i) {
        if (i < val.length()) {
            EEPROM.write(start + i, val[i]);
        } else {
            EEPROM.write(start + i, 0);
        }
    }
}

// -------------------------------------------------------------------
// Load saved settings from EEPROM
// -------------------------------------------------------------------
void config_load_settings() {
    EEPROM.begin();

    EEPROM.get(EEPROM_BEGIN, my_setting);
}

void config_reset() {
    ResetEEPROM();
}