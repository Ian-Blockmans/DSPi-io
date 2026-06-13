
#include "gpio_controls.h"
#include "dac_hw_mute.h"

#include "config.h"            /* PIN_CONFIG_* status codes, NUM_SPDIF_INSTANCES */
#include "flash_storage.h"     /* dir_cache, flash_storage_mark_directory_dirty */
#include "notify.h"            /* notify_param_write */
#include "bulk_params.h"       /* WireBulkParams offsets */

#include "hardware/gpio.h"

#include <stddef.h>
#include <string.h>


extern uint8_t output_pins[];
extern uint8_t i2s_bck_pin;
extern uint8_t i2s_mck_pin;
extern bool    i2s_mck_enabled;
extern uint8_t spdif_rx_pin;

static GpioStatics s = {0};

//local

static void claim_input_pin(uint8_t pin, uint8_t feature) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    switch (feature) {
        case GPIO_FEATURE_MUTE:
            if (s.cfg_mute.active_low == 1){
                gpio_pull_up(pin);
            } else {
                gpio_pull_down(pin);
            }
            s.pin_mute_claimed = true;
            break;
        case GPIO_FEATURE_VOLUME:
            if (s.cfg_volume.active_low == 1){
                gpio_pull_up(pin);
            } else {
                gpio_pull_down(pin);
            }
            s.pin_volume_claimed = true; 
            break;
    }
}

static void release_input_pin(uint8_t pin, uint8_t feature) {
    if (!s.pin_mute_claimed && !s.pin_volume_claimed) return;
    gpio_pull_up(pin);
    switch (feature) {
        case GPIO_FEATURE_MUTE:
            s.pin_mute_claimed = false;
            break;
        case GPIO_FEATURE_VOLUME:
            s.pin_volume_claimed = false; 
            break;
    }
}

static void gpio_set_defaults(void){
    s.cfg_mute.enabled = GPIO_MUTE_ENABLED_DEFAULT;
    s.cfg_mute.active_low = GPIO_MUTE_ACTIVE_LOW_DEFAULT;
    s.cfg_mute.pin = GPIO_IN_PIN;
    s.cfg_volume.enabled = GPIO_VOLUME_ENABLED_DEFAULT;
    s.cfg_volume.active_low = GPIO_VOLUME_ACTIVE_LOW_DEFAULT;
    s.cfg_volume.rotary = GPIO_VOLUME_ROTARY;
    s.cfg_volume.up_a_pin = GPIO_VOLUME_UP_A_PIN;
    s.cfg_volume.down_b_pin = GPIO_VOLUME_DOWN_B_PIN;
}

//global

/* returns a 48 byte array with all the gpio pins that are in use. ends in GPIO_PIN_END(0xfe) if not all pins are in use.*/
uint8_t* get_gpio_in_use(void){
    static uint8_t in_use[GPIO_MAX_PIN+1];
    uint8_t index = 0;
    DacHwMuteConfig hw_mute_config;
    dac_hw_mute_get_config(&hw_mute_config); // get hw mute config
    in_use[index] = i2s_bck_pin;
    index++;
    if (i2s_mck_enabled) {
      in_use[index] = i2s_mck_pin;
      index++;
    }
    in_use[index] = spdif_rx_pin;
    index++;
    in_use[index] = hw_mute_config.pin;
    index++;
    in_use[index] = GPIO_PIN_END;
    return in_use;
}

/* External pin-conflict returns true when the pin is in use */

bool gpio_in_use_conflict(uint8_t pin) {
    uint8_t* in_use = get_gpio_in_use();
    uint8_t index = 0;
    while(in_use[index] != GPIO_PIN_END && index <= GPIO_MAX_PIN) {
        if(pin == in_use[index]){
            return true;
        }  
    }
    return false;
}

void gpio_input_poll(void){
    uint8_t mute = 0;
    if (s.cfg_mute.active_low && !gpio_get(s.cfg_mute.pin) || !s.cfg_mute.active_low && gpio_get(s.cfg_mute.pin)){
        mute = 1;
    } else {
        mute = 0;
    }
    if (mute != s.muted){
        notify_param_write(offsetof(WireBulkParams, user_volume.user_mute),
                           sizeof(uint8_t), &mute);
    }

    //volume ...
}

void gpio_controls_mute_init(const GpioControlsMuteConfig *cfg){
    release_input_pin(cfg->pin, GPIO_FEATURE_MUTE);

    if (!cfg || cfg->enabled == 0) {
        memset(&s.cfg_mute, 0, sizeof(s.cfg_mute));
        s.muted = false;
        return;
    }

    memcpy(&s.cfg_mute, cfg, sizeof(s.cfg_mute));

    if (s.cfg_mute.pin != GPIO_PIN_NONE) {
        claim_input_pin(cfg->pin, GPIO_FEATURE_MUTE);
    }

    s.muted = false;
}

void preset_get_gpio_controls_mute(GpioControlsMuteConfig *out){
    gpio_set_defaults(); // remove later
    if (!out) return;
    memcpy(out, &s.cfg_mute, sizeof(*out));
}

void preset_get_gpio_controls_Volume(GpioControlsVolumeConfig *out){ // move to flash_storage.c
    gpio_set_defaults(); // remove later
    if (!out) return;
    memcpy(out, &s.cfg_volume, sizeof(*out));
}

uint8_t gpio_controls_mute_set_config(const GpioControlsMuteConfig *cfg){
    /* When disabling, just release and zero-out — no other validation
     * needed.  enabled==0 is always accepted. */
    if (cfg->enabled == 0) {
        // zero all static vars
        s.cfg_mute.enabled = 0;
        s.cfg_mute.active_low = 0;
        s.cfg_mute.pin = 0;
        // write to flash preset
        // write to wire
        return PIN_CONFIG_SUCCESS;
    }
    if (cfg->enabled > 1) return PIN_CONFIG_INVALID_OUTPUT;
    if (cfg->active_low > 1) return PIN_CONFIG_INVALID_OUTPUT;
    if (cfg->pin != GPIO_PIN_NONE) {
        if (cfg->pin > GPIO_MAX_PIN) return PIN_CONFIG_INVALID_PIN;
        if (gpio_in_use_conflict(cfg->pin)) return PIN_CONFIG_PIN_IN_USE;
    }
}

uint8_t gpio_controls_volume_set_config(GpioControlsVolumeConfig *cfg){
    if (cfg->enabled == 0) {
        // zero all static vars
        s.cfg_volume.enabled = 0;
        s.cfg_volume.active_low = 0;
        s.cfg_volume.rotary = 0;
        s.cfg_volume.up_a_pin = 0;
        s.cfg_volume.down_b_pin = 0;
        // write to flash preset
        // write to wire
        return PIN_CONFIG_SUCCESS;
    }
    if (cfg->enabled > 1) return PIN_CONFIG_INVALID_OUTPUT;
    if (cfg->active_low > 1) return PIN_CONFIG_INVALID_OUTPUT;
    if (cfg->rotary > 1) return PIN_CONFIG_INVALID_OUTPUT;
    if (cfg->up_a_pin > GPIO_MAX_PIN) return PIN_CONFIG_INVALID_PIN;
    if (cfg->down_b_pin > GPIO_MAX_PIN) return PIN_CONFIG_INVALID_PIN;
}
