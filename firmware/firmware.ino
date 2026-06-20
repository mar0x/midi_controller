#include "pinout.h"
#include "spi.h"
#include "display_1602.h"

#include "encoder.h"
#include "keyboard.h"
#include "stimer.h"
#include "artl/yield.h"

using namespace midi_controller;

const uint16_t CLOCK_PERIOD = 1000;

enum {
    MODE_INIT,
    MODE_NORMAL,
    MODE_SETTINGS_FIRST,
    MODE_SETTINGS_CHANNEL = MODE_SETTINGS_FIRST,
    MODE_SETTINGS_PROG_OUT,
    MODE_SETTINGS_MIDI_FWD,
    MODE_SETTINGS_HIDE_CURSOR,
    MODE_SETTINGS_PROG_START,
#if defined(DEBUG)
    MODE_SETTINGS_USB_DEBUG,
#endif
    MODE_SETTINGS_FACTORY_RESET,
    MODE_SETTINGS_LAST = MODE_SETTINGS_FACTORY_RESET,
    MODE_PROGRAM_SWAP,
    MODE_PROGRAM_MOVE,
    MODE_MIDI_IN_MONITOR,
    MODE_MIDI_OUT_MONITOR,
    MODE_MIDI_DUMP_SEND,
    MODE_MIDI_DUMP_RECV,
    MODE_ABOUT,
    MODE_UPTIME,
    MODE_MAX,
};

struct display_text_t {
    uint8_t row:4;
    uint8_t col:4;
    const char *str;
};

struct mode_desc_t;

struct mode_desc_t {
    uint8_t mode;
    const display_text_t *text;
    void (*enter)(mode_desc_t *mode);
    void (*leave)(mode_desc_t *mode);
    void (*onkey)(mode_desc_t *mode, uint8_t key);
};

void default_mode_enter(mode_desc_t *mode) {
    const display_text_t *t = mode->text;

    while (t && t->str) {
        display_1602::set_cursor(t->row, t->col);
        display_1602::write(t->str);
        ++t;
    }
}

void default_mode_leave(mode_desc_t * /* mode */) {
}

void default_mode_onkey(mode_desc_t * /* mode */, uint8_t /* key */) {
}

display_text_t init_text[] = {
    { 0, 0, "Mode INIT" }, { 0, 0, nullptr },
};

display_text_t normal_text[] = {
    { 0, 0, "Mode NORMAL" }, { 0, 0, nullptr },
};

mode_desc_t mode[] = {
    { MODE_INIT,   init_text,  default_mode_enter },
    { MODE_NORMAL, normal_text , default_mode_enter },
};

namespace midi_controller {

void encoder::on_rotate(int8_t d) {
}

}

void update_clock() {
/*
    if (Serial.dtr()) {
        Serial.print(millis());
        Serial.print(" left ");
        Serial.print(left_btn_pin::read());

        Serial.print(", right ");
        Serial.print(right_btn_pin::read());

        Serial.print(", up ");
        Serial.print(up_btn_pin::read());

        Serial.print(", down ");
        Serial.print(down_btn_pin::read());

        Serial.print(", ok ");
        Serial.println(ok_btn_pin::read());
    }
*/
}

void setup() {
    stimer::setup();

    keyboard::setup();
    encoder::setup();

    ok_led_pin::setup();
    up_led_pin::setup();
    left_led_pin::setup();
    down_led_pin::setup();

    eeprom_cs::setup();
    eeprom_cs::inactive();

    oled_cs::setup();
    oled_cs::inactive();

    spi::init();

    display_1602::begin();
    display_1602::home();
    display_1602::write_pgm(PSTR("Hello PSTR"));

    stimer::schedule_in(STIMER_CLOCK, 0, CLOCK_PERIOD, update_clock);

    Serial.begin(115200);
}

void loop() {
    clk_t::clock_source::update();

    stimer::update();

    keyboard::update();
    encoder::update();

    // ok_led_pin::write(!ok_btn_pin::read());
    up_led_pin::write(!up_btn_pin::read());
    down_led_pin::write(!down_btn_pin::read());
    left_led_pin::write(!left_btn_pin::read());

    artl::yield();
}
