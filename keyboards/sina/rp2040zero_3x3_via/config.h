#pragma once

/* --- OLED (SSD1306 over I2C) --- */
#define OLED_DRIVER_ENABLE
#define OLED_TIMEOUT 60000
#define OLED_BRIGHTNESS 96

/* Use I2C1 but remap pins to GP0/GP1 (your wiring) */
#define I2C_DRIVER I2CD1
#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1

/* --- RGB (WS2812 / NeoPixel) --- */
#define RGB_DI_PIN GP11
#define RGBLED_NUM 9
#define RGBLIGHT_LIMIT_VAL 90
#define RGBLIGHT_SLEEP

/* --- Encoder button (NOT in matrix) --- */
#define ENCODER_BTN_PIN GP10

/* --- RP2040 quality-of-life --- */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U
