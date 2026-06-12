
#define GPIO_PIN_NONE 0xFFu

#define GPIO_MAX_PIN 48 // rp2350 has 48 gpio pins, rp 2040 has 30 pins

// defaults
#define GPIO_MUTE_ENABLED_DEFAULT 1
#define GPIO_MUTE_ACTIVE_LOW_DEFAULT 0
#define GPIO_MUTE_IN_PIN 17
#define GPIO_VOLUME_ENABLED_DEFAULT 1
#define GPIO_VOLUME_ACTIVE_LOW_DEFAULT 0
#define GPIO_VOLUME_ROTARY 0
#define GPIO_VOLUME_UP_A_PIN 18
#define GPIO_VOLUME_DOWN_B_PIN 19

typedef struct __attribute__((packed)) {
    uint8_t  mute_enabled;           // mute button/switch. 0 = feature off, 1 = on
    uint8_t  mute_active_low;        // 1 = pull pin LOW to mute, 0 = pull HIGH
    uint8_t  mute_in_pin;               // GPIO; 0xFF = no pin
    uint8_t  volume_enabled;         // physical volume control. 0 = feature off, 1 = on
    uint8_t  volume_active_low;      // 1 = pull pin LOW to adjust volume, 0 = pull HIGH
    uint8_t  volume_rotary;                 // 1 = rotary encoder, 0 = 2 buttons. ignore volume_active_low when using rotary encoder.
    uint8_t  volume_up_a_pin;        // GPIO; 0xFF = no pin. pin a/clk for rotary encoder
    uint8_t  volume_down_b_pin;      // GPIO; 0xFF = no pin. pin b/dt for rotary encoder
    uint8_t  reserved[8];            // zero-fill
} GpioControlsConfig;