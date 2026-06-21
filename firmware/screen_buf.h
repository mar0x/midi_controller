#pragma once

#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>

namespace midi_controller {

template<typename DISP, uint8_t LINES = 2, uint8_t COLUMNS = 16>
struct screen_buf_t {
    using display_type = DISP;

    static char buf[LINES * COLUMNS];
    static uint8_t pos;

    static void setup();

    static void clear();
    static void clear(uint8_t line);

    static void update(uint8_t col, uint8_t row, uint8_t size);
    static void update(uint8_t col, uint8_t row);
    static void update();

    static void write(uint8_t col, uint8_t row, char c);

    static void set_cursor(uint8_t col, uint8_t row);

    static uint8_t write(char c);
    static uint8_t write(const char *c, uint8_t size);
    static uint8_t write(const char *c);
    static uint8_t write_pgm(const char *c, uint8_t size);
    static uint8_t write_pgm(const char *d);
};

}

namespace midi_controller {

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
char screen_buf_t<DISP, LINES, COLUMNS>::buf[LINES * COLUMNS];

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
uint8_t screen_buf_t<DISP, LINES, COLUMNS>::pos;

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::setup() {
    display_type::setup();
    display_type::home();
    pos = 0;
    memset(buf, ' ', sizeof(buf));
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::clear() {
    pos = 0;
    memset(buf, ' ', sizeof(buf));
    display_type::clear();
    display_type::home();
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::clear(uint8_t line) {
    memset(buf + line * COLUMNS, ' ', COLUMNS);
    update(0, line, COLUMNS);
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::update(uint8_t col, uint8_t row, uint8_t size) {
    pos = row * COLUMNS + col;
    display_type::set_cursor(col, row);
    display_type::write(buf + pos, size);
    pos += size;
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::update(uint8_t col, uint8_t row) {
    pos = row * COLUMNS + col;
    display_type::set_cursor(col, row);
    display_type::write(buf[pos]);
    pos += 1;
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::write(uint8_t col, uint8_t row, char c) {
    set_cursor(col, row);
    write(c);
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
void screen_buf_t<DISP, LINES, COLUMNS>::set_cursor(uint8_t col, uint8_t row) {
    pos = row * COLUMNS + col;
    display_type::set_cursor(col, row);
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
uint8_t screen_buf_t<DISP, LINES, COLUMNS>::write(char c) {
    buf[pos++] = c;
    return display_type::write(c);
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
uint8_t screen_buf_t<DISP, LINES, COLUMNS>::write(const char *c, uint8_t size) {
    if (size > sizeof(buf) - pos) {
        size = sizeof(buf) - pos;
    }

    memcpy(buf + pos, c, size);
    pos += size;

    return display_type::write(c, size);
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
uint8_t screen_buf_t<DISP, LINES, COLUMNS>::write(const char *c) {
    return write(c, strlen(c));
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
uint8_t screen_buf_t<DISP, LINES, COLUMNS>::write_pgm(const char *c, uint8_t size) {
    if (size > sizeof(buf) - pos) {
        size = sizeof(buf) - pos;
    }

    memcpy_P(buf + pos, c, size);
    pos += size;

    return display_type::write(c, size);
}

template<typename DISP, uint8_t LINES, uint8_t COLUMNS>
uint8_t screen_buf_t<DISP, LINES, COLUMNS>::write_pgm(const char *c) {
    char b;
    uint8_t size = 0;

    while (pos < sizeof(buf)) {
        b = pgm_read_byte(c++);
        if (b) {
            buf[pos++] = b;
            ++size;
        } else {
            break;
        }
    }

    return display_type::write(buf + pos - size, size);
}

}
