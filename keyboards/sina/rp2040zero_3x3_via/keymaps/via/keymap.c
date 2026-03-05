#include QMK_KEYBOARD_H
#include "oled_driver.h"

/* ---------------- Layers ---------------- */
enum layer_names {
    _BASE = 0,
    _FN
};

/* ---------------- RGB layer colors ----------------
   HSV values are 0-255. */
static void set_layer_rgb(uint8_t layer) {
    if (!rgblight_is_enabled()) return;

    switch (layer) {
        case _BASE:
            rgblight_sethsv_noeeprom(170, 255, 120); // cyan/blue-ish
            break;
        case _FN:
            rgblight_sethsv_noeeprom(10, 255, 140);  // orange/red-ish
            break;
        default:
            rgblight_sethsv_noeeprom(0, 0, 0);
            break;
    }
}

/* ---------------- Small startup animation ---------------- */
static void startup_anim(void) {
    if (!rgblight_is_enabled()) {
        rgblight_enable_noeeprom();
    }

    // Quick rainbow sweep, then settle to layer color
    for (uint8_t h = 0; h < 255; h += 9) {
        rgblight_sethsv_noeeprom(h, 255, 90);
        wait_ms(20);
    }
    set_layer_rgb(get_highest_layer(layer_state));
}

/* ---------------- Encoder button (not in matrix) ---------------- */
static bool encoder_btn_last = true; // pullup -> idle high
static uint16_t encoder_btn_debounce = 0;

static void encoder_btn_scan(void) {
    bool now = readPin(ENCODER_BTN_PIN); // true=high, false=low
    uint16_t t = timer_read();

    if (now != encoder_btn_last) {
        encoder_btn_debounce = t;
        encoder_btn_last = now;
        return;
    }

    // stable state for 20ms
    if (timer_elapsed(encoder_btn_debounce) < 20) return;

    // detect press (high -> low)
    static bool was_pressed = false;
    bool pressed = (now == false);

    if (pressed && !was_pressed) {
        rgblight_toggle_noeeprom();
        if (rgblight_is_enabled()) {
            set_layer_rgb(get_highest_layer(layer_state));
        }
    }
    was_pressed = pressed;
}

/* ---------------- OLED: last key info ---------------- */
typedef struct {
    bool     valid;
    bool     pressed;
    uint8_t  row;
    uint8_t  col;
    uint16_t keycode;
} last_key_t;

static last_key_t g_last = {0};

static const char *kc_to_name(uint16_t kc) {
    switch (kc) {
        case KC_NO:   return "KC_NO";
        case KC_TRNS: return "KC_TRNS";
        case KC_ESC:  return "ESC";
        case KC_TAB:  return "TAB";
        case KC_ENT:  return "ENTER";
        case KC_BSPC: return "BSPC";
        case KC_SPC:  return "SPACE";
        case KC_DEL:  return "DEL";
        case KC_UP:   return "UP";
        case KC_DOWN: return "DOWN";
        case KC_LEFT: return "LEFT";
        case KC_RGHT: return "RIGHT";
        case KC_VOLU: return "VOL+";
        case KC_VOLD: return "VOL-";
        case KC_MUTE: return "MUTE";
        case QK_BOOT: return "BOOT";
        default:      return NULL;
    }
}

static void oled_write_u8(uint8_t v) {
    char buf[4];
    buf[0] = '0' + (v / 10);
    buf[1] = '0' + (v % 10);
    buf[2] = 0;
    oled_write(buf, false);
}

static void oled_write_hex16(uint16_t v) {
    char buf[7];
    const char *hex = "0123456789ABCDEF";
    buf[0] = '0'; buf[1] = 'x';
    buf[2] = hex[(v >> 12) & 0xF];
    buf[3] = hex[(v >> 8)  & 0xF];
    buf[4] = hex[(v >> 4)  & 0xF];
    buf[5] = hex[(v >> 0)  & 0xF];
    buf[6] = 0;
    oled_write(buf, false);
}

/* Capture matrix position + keycode */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.key.row < MATRIX_ROWS && record->event.key.col < MATRIX_COLS) {
        g_last.valid   = true;
        g_last.pressed = record->event.pressed;
        g_last.row     = record->event.key.row;
        g_last.col     = record->event.key.col;
        g_last.keycode = keycode;
    }
    return true;
}

bool oled_task_user(void) {
    oled_clear();

    // Line 1: layer + RGB
    oled_write("L:", false);
    oled_write_u8(get_highest_layer(layer_state));
    oled_write("  RGB:", false);
    oled_write(rgblight_is_enabled() ? "ON" : "OFF", false);
    oled_write("\n", false);

    // Line 2-4: last key
    oled_write("Last:\n", false);

    if (!g_last.valid) {
        oled_write("(none)\n", false);
        return false;
    }

    oled_write("pos r", false);
    oled_write_u8(g_last.row);
    oled_write(" c", false);
    oled_write_u8(g_last.col);
    oled_write(g_last.pressed ? "  dn\n" : "  up\n", false);

    oled_write("kc ", false);
    oled_write_hex16(g_last.keycode);
    oled_write("\n", false);

    const char *name = kc_to_name(g_last.keycode);
    oled_write("fn ", false);
    if (name) {
        oled_write(name, false);
    } else {
        oled_write("(raw)", false);
    }
    oled_write("\n", false);

    return false;
}

/* ---------------- Encoder rotation ----------------
   Default: volume up/down */
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index != 0) return false;
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return false;
}

/* ---------------- Layer -> RGB color ---------------- */
layer_state_t layer_state_set_user(layer_state_t state) {
    set_layer_rgb(get_highest_layer(state));
    return state;
}

/* ---------------- Init ---------------- */
void keyboard_post_init_user(void) {
    // Encoder button pin setup (pull-up)
    setPinInputHigh(ENCODER_BTN_PIN);

    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);

    startup_anim();
}

/* Scan hook: handle encoder button debounce */
void matrix_scan_user(void) {
    encoder_btn_scan();
}

/* ---------------- Keymaps (VIA) ---------------- */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /* BASE
     *  [ESC] [TAB] [MO1]
     *  [  1] [  2] [  3]
     *  [  4] [  5] [BSPC]
     */
    [_BASE] = LAYOUT_3x3(
        KC_ESC,  KC_TAB,  MO(_FN),
        KC_1,    KC_2,    KC_3,
        KC_4,    KC_5,    KC_BSPC
    ),

    /* FN
     *  [BOOT] [DEL ] [TO0]
     *  [LEFT] [DOWN] [RGHT]
     *  [MUTE] [SPC ] [ENT ]
     */
    [_FN] = LAYOUT_3x3(
        QK_BOOT, KC_DEL,  TO(_BASE),
        KC_LEFT, KC_DOWN, KC_RGHT,
        KC_MUTE, KC_SPC,  KC_ENT
    )
};
