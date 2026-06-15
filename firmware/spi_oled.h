
#pragma once

#include <inttypes.h>
#include <SPI.h>
#include "Print.h"
#include "artl/digital_out.h"

enum {
// commands
    LCD_CLEARDISPLAY = 0x01,
    LCD_RETURNHOME = 0x02,
    LCD_ENTRYMODESET = 0x04,
    LCD_DISPLAYCONTROL = 0x08,
    LCD_CURSORSHIFT = 0x10,
    LCD_FUNCTIONSET = 0x20,
    LCD_SETCGRAMADDR = 0x40,
    LCD_SETDDRAMADDR = 0x80,

// flags for display entry mode
    LCD_ENTRYRIGHT = 0x00,
    LCD_ENTRYLEFT = 0x02,
    LCD_ENTRYSHIFTINCREMENT = 0x01,
    LCD_ENTRYSHIFTDECREMENT = 0x00,

// flags for display on/off control
    LCD_DISPLAYON = 0x04,
    LCD_DISPLAYOFF = 0x00,
    LCD_CURSORON = 0x02,
    LCD_CURSOROFF = 0x00,
    LCD_BLINKON = 0x01,
    LCD_BLINKOFF = 0x00,

// flags for display/cursor shift
    LCD_DISPLAYMOVE = 0x08,
    LCD_CURSORMOVE = 0x00,
    LCD_MOVERIGHT = 0x04,
    LCD_MOVELEFT = 0x00,

// flags for function set
    LCD_8BITMODE = 0x10,
    LCD_4BITMODE = 0x00,
    LCD_2LINE = 0x08,
    LCD_1LINE = 0x00,
    LCD_5x10DOTS = 0x04,
    LCD_5x8DOTS = 0x00
};

struct spi_oled {
    using cs = artl::digital_out< artl::port::B, 6 >;
    using ss = artl::digital_out< artl::port::B, 0 >;

    void begin(uint8_t, uint8_t) {
        ss::setup();

        cs::setup();
        cs::high();

        SPI.begin();

        // finally, set # lines, font size, etc.
        command(LCD_FUNCTIONSET | LCD_2LINE | LCD_8BITMODE);

        delay(100);

        // turn the display on with no cursor or blinking default
        display_control_ = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
        display();

        // clear it off
        clear();

        // Initialize to default text direction (for romance languages)
        display_mode_ = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
        // set the entry mode
        command(LCD_ENTRYMODESET | display_mode_);

        delay(100);
    }

    void clear() {
        command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    }

    void home() {
        command(LCD_RETURNHOME);  // set cursor position to zero
    }

    void noDisplay() {
        display_control_ &= ~LCD_DISPLAYON;
        command(LCD_DISPLAYCONTROL | display_control_);
    }

    void display() {
        display_control_ |= LCD_DISPLAYON;
        command(LCD_DISPLAYCONTROL | display_control_);
    }

    void noBlink() {
        display_control_ &= ~LCD_BLINKON;
        command(LCD_DISPLAYCONTROL | display_control_);
    }

    void blink() {
        display_control_ |= LCD_BLINKON;
        command(LCD_DISPLAYCONTROL | display_control_);
    }

    void noCursor() {
        display_control_ &= ~LCD_CURSORON;
        command(LCD_DISPLAYCONTROL | display_control_);
    }

    void cursor() {
        display_control_ |= LCD_CURSORON;
        command(LCD_DISPLAYCONTROL | display_control_);
    }

    void scrollDisplayLeft() {
        command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
    }

    void scrollDisplayRight() {
        command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
    }

    void leftToRight() {
        display_mode_ |= LCD_ENTRYLEFT;
        command(LCD_ENTRYMODESET | display_mode_);
    }

    void rightToLeft() {
        display_mode_ &= ~LCD_ENTRYLEFT;
        command(LCD_ENTRYMODESET | display_mode_);
    }

    void autoscroll() {
        display_mode_ |= LCD_ENTRYSHIFTINCREMENT;
        command(LCD_ENTRYMODESET | display_mode_);
    }

    void noAutoscroll() {
        display_mode_ &= ~LCD_ENTRYSHIFTINCREMENT;
        command(LCD_ENTRYMODESET | display_mode_);
    }

    void createChar(uint8_t location, uint8_t charmap[]) {
        location &= 0x7; // we only have 8 locations 0-7
        command(LCD_SETCGRAMADDR | (location << 3));
        write(charmap, 8);
    }

    void setCursor(uint8_t col, uint8_t row) {
        if (row >= 2) { row = 1; }
        if (col >= 16) { col = 15; }

        command(LCD_SETDDRAMADDR | (col + row * 0x40));
    }

    struct transfer {
        transfer() { cs::low(); }
        ~transfer() { cs::high(); }
    };

    size_t write(uint8_t d) {
        transfer t;

        SPI.transfer(0x80 | (d >> 2));
        SPI.transfer(d << 6);

        return 1;
    }

    size_t write(char b) {
        return write((uint8_t) b);
    }

    size_t write(const uint8_t *b, size_t size) {
        transfer t;

        SPI.transfer(0x80 | (b[0] >> 2));

        for (size_t i = 0; i < size - 1; i++) {
            SPI.transfer( (b[i] << 6) | (b[i + 1] >> 2) );
        }

        SPI.transfer(b[size - 1] << 6);

        return size;
    }

    size_t write(const char *b, size_t size) {
        return write((const uint8_t *) b, size);
    }

    void command(uint8_t cmd) {
        transfer t;

        SPI.transfer(cmd >> 2);
        SPI.transfer(cmd << 6);
    }

private:
    uint8_t display_control_;
    uint8_t display_mode_;
};
