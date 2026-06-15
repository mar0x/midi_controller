#include "spi.h"

#ifndef HAVE_SPI
#define HAVE_SPI 2
#endif

#if HAVE_SPI == 2

#if defined(ARDUINO_ARCH_AVR)
#include <avr/io.h>
#endif

#include "artl/crit_sec.h"
#include "pinout.h"

namespace {

midi_controller::spi::callback_t cur_cb;
midi_controller::spi::callback_t next_cb;

const uint8_t bps_denominator[8] = {4, 16, 64, 128, 2, 8, 32, 64};
const uint8_t bps_rshift[8] = {2, 4, 6, 7, 1, 3, 5, 6};
const uint8_t rate_mask = (1 << SPR1) | (1 << SPR0);

volatile uint8_t wait_ok;

void spi_wait_cb(uint8_t w)
{
    if (w == 0) { wait_ok = 1; }
}

}

namespace midi_controller {

void spi::init()
{
    spi_mosi::output();
    spi_sck::output();
    spi_miso::input();

    spi_ss::output();

    // Enable SPI, Master, clock rate f_osc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    // clear double speed
    SPSR &= ~(1 << SPI2X);
}

void spi::rate(uint32_t r)
{
    uint8_t i;
    artl::crit_sec cs;

    for (i = 0; i < 8; i++) {
        if (((uint32_t)F_CPU >> bps_rshift[i]) == r) {
            break;
        }
    }

    if (i < 8) {
        if (i < 4) {
            SPSR &= ~(1 << SPI2X);
        } else {
            SPSR |= (1 << SPI2X);
        }
        SPCR = (SPCR & ~rate_mask) | (i & ((1 << SPR1) | (1 << SPR0)));
    }
}

uint16_t spi::msec2bytes(uint16_t msec)
{
    uint8_t r = (SPSR & (1 << SPI2X) ? 0x04 : 0x00) | (SPCR & rate_mask);
    return (F_CPU >> (bps_rshift[r] + 3)) * msec / 1000;
}

uint8_t spi::transfer(uint8_t b)
{
    SPDR = b;
    while (!(SPSR & (1 << SPIF)))
      ;
    return SPDR;
}

void spi::acquire(callback_t cb)
{
    artl::crit_sec cs;

    if (cur_cb) {
        if (cur_cb == cb) {
            cur_cb(0xFE);
        } else {
            next_cb = cb;
            cur_cb(0xFF);
        }
        return;
    }

    cur_cb = cb;

    cur_cb(0);
}

void spi::acquire()
{
    wait_ok = 0;

    spi::acquire(spi_wait_cb);

    while (!wait_ok) { }
}

void spi::release()
{
    spi::int_off();

    if (next_cb) {
        cur_cb = next_cb;
        next_cb = 0;

        cur_cb(0);
    } else {
        cur_cb = 0;
    }
}

}

ISR(SPI_STC_vect)
{
    if (cur_cb) {
        cur_cb(SPI_STC_vect_num);
    }
}

#elif HAVE_SPI == 1

#include <SPI.h>

namespace midi_controller {

void spi::init()
{
    SPI.begin();
}

uint8_t spi::transfer(uint8_t b)
{
    return SPI.transfer(b);
}

}

#else

namespace midi_controller {

void spi::init() { }

}

#endif

