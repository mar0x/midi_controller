#pragma once

#include <stdint.h>

namespace midi_controller {

struct lcd {
    enum {
        LCD_LINES = 2,
        LCD_COLUMNS = 16,
    };

    static void buf_clear();
    static void buf_clear(uint8_t line);
    static void setup();
    static void update(uint8_t col, uint8_t row, uint8_t size);
    static void update(uint8_t col, uint8_t row);
    static void update();
    static void write(uint8_t col, uint8_t row, uint8_t c);
};

}
