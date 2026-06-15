#include <Arduino.h>
#include "display_1602.h"
#include "bits/lcd.h"
#include "spi.h"
#include "pinout.h"

namespace {

struct transfer_scope {
    transfer_scope() { midi_controller::oled_cs::active(); }
    ~transfer_scope() { midi_controller::oled_cs::inactive(); }
};

uint8_t control_;
uint8_t mode_;

uint8_t bit_set(uint8_t &v, uint8_t mask, bool enable) {
    if (enable) {
        v |= mask;
    } else {
        v &= ~mask;
    }

    return v;
}

}

namespace midi_controller {

void display_1602::begin() {
    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | LCD_2LINE | LCD_8BITMODE);

    delay(100);

    // turn the display on with no cursor or blinking default
    control_ = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    command(control_);

    // clear it off
    clear();

    // Initialize to default text direction (for romance languages)
    mode_ = LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(mode_);

    delay(100);
}

void display_1602::clear() {
    command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
}

void display_1602::home() {
    command(LCD_RETURNHOME);  // set cursor position to zero
}

void display_1602::display(bool enable) {
    command(bit_set(control_, LCD_DISPLAYON, enable));
}

void display_1602::blink(bool enable) {
    command(bit_set(control_, LCD_BLINKON, enable));
}

void display_1602::cursor(bool enable) {
    command(bit_set(control_, LCD_CURSORON, enable));
}

void display_1602::left_to_right(bool enable) {
    command(bit_set(mode_, LCD_ENTRYLEFT, enable));
}

void display_1602::autoscroll(bool enable) {
    command(bit_set(mode_, LCD_ENTRYSHIFTINCREMENT, enable));
}

void display_1602::scroll_left() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void display_1602::scroll_right() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void display_1602::create_char(uint8_t location, const uint8_t *charmap) {
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    write(charmap, 8);
}

void display_1602::set_cursor(uint8_t col, uint8_t row) {
    if (row >= 2) { row = 1; }
    if (col >= 16) { col = 15; }

    command(LCD_SETDDRAMADDR | (col + row * 0x40));
}

uint8_t display_1602::write(uint8_t d) {
    transfer_scope ts;

    spi::send(0x80 | (d >> 2));
    spi::send(d << 6);

    return 1;
}

uint8_t display_1602::write(char c) {
    return write((uint8_t) c);
}

uint8_t display_1602::write(const uint8_t *d, uint8_t size) {
    if (size != 0) {
        transfer_scope ts;

        spi::send(0x80 | (d[0] >> 2));

        for (uint8_t i = 0; i < size - 1; i++) {
            spi::send( (d[i] << 6) | (d[i + 1] >> 2) );
        }

        spi::send(d[size - 1] << 6);
    }

    return size;
}

uint8_t display_1602::write(const char *c, uint8_t size) {
    return write((const uint8_t *) c, size);
}

uint8_t display_1602::write(const char *d) {
    uint8_t di, di1, i = 0;

    di = d[0];
    if (di != 0) {
        transfer_scope ts;

        spi::send(0x80 | (di >> 2));

        for (i = 1; i < 255; i++) {
            di1 = d[i];
            if (di1 != 0) {
                spi::send( (di << 6) | (di1 >> 2) );
                di = di1;
            } else {
                break;
            }
        }

        spi::send(di << 6);
    }

    return i;
}

uint8_t display_1602::write_pgm(const uint8_t *d, uint8_t size) {
    if (size != 0) {
        transfer_scope ts;
        uint8_t di, di1;

        di = pgm_read_byte(d);
        spi::send(0x80 | (di >> 2));

        for (uint8_t i = 0; i < size - 1; i++) {
            di1 = pgm_read_byte(d + i + 1);
            spi::send( (di << 6) | (di1 >> 2) );
            di = di1;
        }

        spi::send(di << 6);
    }

    return size;
}

uint8_t display_1602::write_pgm(const char *c, uint8_t size) {
    return write_pgm((const uint8_t *) c, size);
}

uint8_t display_1602::write_pgm(const char *d) {
    uint8_t di, di1, i = 0;

    di = pgm_read_byte(d);
    if (di != 0) {
        transfer_scope ts;

        spi::send(0x80 | (di >> 2));

        for (i = 1; i < 255; i++) {
            di1 = pgm_read_byte(d + i);
            if (di1 != 0) {
                spi::send( (di << 6) | (di1 >> 2) );
                di = di1;
            } else {
                break;
            }
        }

        spi::send(di << 6);
    }

    return i;
}

void display_1602::command(uint8_t cmd) {
    transfer_scope ts;

    spi::send(cmd >> 2);
    spi::send(cmd << 6);
}

}
