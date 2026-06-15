
#pragma once

#include <SPI.h>

#include "artl/digital_out.h"
#include "artl/yield.h"

class spi_eeprom {
    using cs = artl::digital_out< artl::port::D, 5 >;

    enum cmd {
        READ  = 0x03,
        WRITE = 0x02,
        RDSR  = 0x05,
        WRSR  = 0x01,
        WREN  = 0x06,
        WRDI  = 0x04,
    };

    enum sr_mask {
        RDY  = (1 << 0),
        WEN  = (1 << 1),
        BP0  = (1 << 2),
        BP1  = (1 << 3),
        WPEN = (1 << 7),
    };

    struct transfer {
        transfer() { cs::low(); }
        ~transfer() { cs::high(); }
    };

public:
    static void setup() {
        cs::setup();
        cs::high();
    }

    static constexpr uint16_t length() { return 0x2000; }

    static uint8_t status() {
        transfer scope;

        SPI.transfer(RDSR);
        return SPI.transfer(0);
    }

    static void wait_ready() {
        while ((status() & RDY) != 0) {
            yield();
        }
    }

    static void write_enable() {
        transfer scope;

        SPI.transfer(WREN);
    }

    static void write_disable() {
        transfer scope;

        SPI.transfer(WRDI);
    }

    template<typename T>
    static void get(uint16_t addr, T &t) {
        wait_ready();

        {
            transfer scope;

            SPI.transfer(READ);
            SPI.transfer(addr >> 8);
            SPI.transfer(addr & 0xFF);

            uint8_t *p = (uint8_t*) &t;
            for (uint8_t i = sizeof(T); i; --i) {
                *p++ = SPI.transfer(0);
            }
        }
    }

    template<typename T>
    static void put(uint16_t addr, const T &t) {
        wait_ready();
        write_enable();

        {
            transfer scope;

            SPI.transfer(WRITE);
            SPI.transfer(addr >> 8);
            SPI.transfer(addr & 0xFF);

            uint8_t *p = (uint8_t*) &t;
            for (uint8_t i = sizeof(T); i; --i) {
                SPI.transfer(*p++);
            }
        }
    }
};
