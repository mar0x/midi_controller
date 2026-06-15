#pragma once

#include <inttypes.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/io.h>
#endif

namespace midi_controller {

struct spi {
    using callback_t = void (*)(uint8_t);

    static void init();

    static uint8_t transfer(uint8_t b);

    static void send(uint8_t b) { transfer(b); }

    static uint8_t recv() { return transfer(0xFF); }

    static void rate(uint32_t r);
    static uint16_t msec2bytes(uint16_t msec);

    static uint8_t data() { return SPDR; }
    static void data(uint8_t b) { SPDR = b; }

    static void int_on() { SPCR |= (1 << SPIE); }
    static void int_off() { SPCR &= ~(1 << SPIE); }

    static void acquire();
    static void acquire(callback_t cb);
    static void release();
};

}
