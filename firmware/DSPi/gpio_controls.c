
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

static GpioControlsConfig s_cfg;
static uint8_t s_mute = 0;
static bool s_pin_claimed = false;

/* returns a 47 byte array with all the gpio pins that are in use. ends in GPIO_PIN_NONE(0xff) if not all pins are in use.*/
uint8_t* gpio_in_use_get(void){
    uint8_t in_use[GPIO_MAX_PIN];
    uint8_t index;
    DacHwMuteConfig mute_config;
    dac_hw_mute_get_config(&mute_config);
    memcpy(&in_use, &output_pins, sizeof(output_pins));
    index = sizeof(output_pins);
    in_use[index] = i2s_bck_pin;
    index++;
    if (i2s_mck_enabled) {
      in_use[index] = i2s_mck_pin;
      index++;
    }
    in_use[index] = spdif_rx_pin;
    index++;
    in_use[index] = mute_config.pin;
    index++;
    in_use[index] = GPIO_PIN_NONE;
    return in_use;
}


/* External pin-conflict returns true when the pin is in use */

bool gpio_in_use_conflict(uint8_t pin) {
    uint8_t in_use[GPIO_MAX_PIN];
    &in_use = get_gpio_in_use();
    uint8_t index = 0;
    while(in_use[index] != GPIO_PIN_NONE && index <= GPIO_MAX_PIN-1) {
        if(pin == in_use[index]){
            return true;
        }  
    }
    return false;
}

void dac_hw_mute_button_poll(void){
    uint8_t mute = 0;
    if (cfg.mute_active_low && !gpio_get(cfg.mute_in_pin) || !cfg.mute_active_low && gpio_get(cfg.mute_in_pin)){
        mute = 1;
    } else {
        mute = 0;
    }
    if (mute != s_mute){
        notify_param_write(offsetof(WireBulkParams, user_volume.user_mute),
               sizeof(uint8_t), &mute);
    }  
}

static void claim_input_pin(uint8_t pin) {
    gpio_init(pin);
    gpio_put(pin, false);
    gpio_set_dir(pin, GPIO_IN);
    s_pin_claimed = true;
}

void gpio_controls_init(GpioControlsConfig *cfg){

}

void preset_get_gpio_controls(GpioControlsConfig *cfg){
    gpio_set_defaults(); // remove later
    if (!out) return;
    memcpy(out, &s_cfg, sizeof(*out));
}

void gpio_set_defaults(void){
    s_cfg.mute_enabled = GPIO_MUTE_ENABLED_DEFAULT
    s_cfg.mute_active_low = GPIO_MUTE_ACTIVE_LOW_DEFAULT
    s_cfg.mute_in_pin = GPIO_MUTE_IN_PIN
    s_cfg.volume_enabled = GPIO_VOLUME_ENABLED_DEFAULT
    s_cfg.volume_active_low = GPIO_VOLUME_ACTIVE_LOW_DEFAULT
    s_cfg.volume_rotary = GPIO_VOLUME_ROTARY
    s_cfg.volume_up_a_pin = GPIO_VOLUME_UP_A_PIN
    s_cfg.volume_down_b_pin = GPIO_VOLUME_DOWN_B_PIN
}