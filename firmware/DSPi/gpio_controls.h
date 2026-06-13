// include
#include <stdint.h>
#include <stdbool.h>

#define GPIO_PIN_NONE 0xFFu
#define GPIO_PIN_END 0xFEu

#if PICO_RP2350  // rp2350 has 48 gpio pins, rp 2040 has 30 pins
    #define GPIO_MAX_PIN 47
#else
    #define GPIO_MAX_PIN 29
#endif

// defaults
#define GPIO_MUTE_ENABLED_DEFAULT 1
#define GPIO_MUTE_ACTIVE_LOW_DEFAULT 0
#define GPIO_MUTE_IN_PIN 17

#define GPIO_VOLUME_ENABLED_DEFAULT 1
#define GPIO_VOLUME_ACTIVE_LOW_DEFAULT 0
#define GPIO_VOLUME_ROTARY 0
#define GPIO_VOLUME_UP_A_PIN 18
#define GPIO_VOLUME_DOWN_B_PIN 19

// feature defines
#define GPIO_FEATURE_MUTE 0
#define GPIO_FEATURE_VOLUME 1

typedef struct __attribute__((packed)) {
    uint8_t  enabled;           // mute button/switch. 0 = feature off, 1 = on
    uint8_t  active_low;        // 1 = pull pin LOW to mute, 0 = pull HIGH
    uint8_t  pin;               // GPIO; 0xFF = no pin
    uint8_t  reserved[5];       // zero-fill
} GpioControlsMuteConfig;

typedef struct __attribute__((packed)) {
    uint8_t  enabled;           // physical volume control. 0 = feature off, 1 = on
    uint8_t  active_low;        // 1 = pull pin LOW to adjust volume, 0 = pull HIGH
    uint8_t  rotary;            // 1 = rotary encoder, 0 = 2 buttons. ignore volume_active_low when using rotary encoder.
    uint8_t  up_a_pin;          // GPIO; 0xFF = no pin. pin a/clk for rotary encoder
    uint8_t  down_b_pin;        // GPIO; 0xFF = no pin. pin b/dt for rotary encoder
    uint8_t  reserved[3];       // zero-fill
} GpioControlsVolumeConfig;

typedef struct {
    uint8_t                  pin_mute_claimed;
    uint8_t                  muted;
    GpioControlsMuteConfig   cfg_mute;
    uint8_t                  pin_volume_claimed;
    GpioControlsVolumeConfig cfg_volume;
} GpioStatics;

// functions
void gpio_controls_mute_init(const GpioControlsMuteConfig *cfg);
void gpio_input_poll(void);
void preset_get_gpio_controls_mute(GpioControlsMuteConfig *out);
uint8_t* gpio_in_use_get(void);
uint8_t gpio_controls_mute_set_config(const GpioControlsMuteConfig *cfg);
