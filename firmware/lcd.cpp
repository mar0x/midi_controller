#include "lcd.h"
#include <string.h>
#include "display_1602.h"

namespace midi_controller {

namespace {

char buf[lcd::LCD_LINES * lcd::LCD_COLUMNS];

}

using display = display_1602;

void lcd::buf_clear() {
    memset(buf, ' ', sizeof(buf));
}

void lcd::buf_clear(uint8_t line) {
    memset(buf + line * LCD_COLUMNS, ' ', LCD_COLUMNS);
}

void lcd::setup() {
    display::begin();
}

void lcd::update(uint8_t col, uint8_t row, uint8_t size) {
    display::set_cursor(col, row);
    display::write(buf + row * LCD_COLUMNS + col, size);
}

void lcd::update(uint8_t col, uint8_t row) {
    display::set_cursor(col, row);
    display::write(buf[row * LCD_COLUMNS + col]);
}

void lcd::update() {
    for (uint8_t i = 0; i < LCD_LINES; i++) {
        update(0, i, LCD_COLUMNS);
    }
}

void lcd::write(uint8_t col, uint8_t row, uint8_t c) {
    display::set_cursor(col, row);
    display::write(c);
}

}
