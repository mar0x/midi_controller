#pragma once

#include <inttypes.h>

namespace midi_controller {

struct display_1602 {
    static void setup();

    static void clear();
    static void home();

    static void display(bool enable = true);
    static void blink(bool enable = true);
    static void cursor(bool enable = true);
    static void left_to_right(bool enable = true);
    static void autoscroll(bool enable = true);

    static void scroll_left();
    static void scroll_right();

    static void create_char(uint8_t location, const uint8_t *charmap);

    static void set_cursor(uint8_t col, uint8_t row);

    static uint8_t write(uint8_t d);
    static uint8_t write(char c);
    static uint8_t write(const uint8_t *d, uint8_t size);
    static uint8_t write(const char *c, uint8_t size);
    static uint8_t write(const char *c);
    static uint8_t write_pgm(const uint8_t *d, uint8_t size);
    static uint8_t write_pgm(const char *c, uint8_t size);
    static uint8_t write_pgm(const char *d);

    static void command(uint8_t cmd);
};

}
