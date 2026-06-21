#include "spi_eeprom.h"

#include "spi.h"
#include "pinout.h"

#include "artl/yield.h"

namespace {

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

using midi_controller::eeprom_cs;
using midi_controller::spi;

struct transfer_scope {
    transfer_scope() { eeprom_cs::active(); }
    ~transfer_scope() { eeprom_cs::inactive(); }

    static void setup() {
        eeprom_cs::setup();
        eeprom_cs::inactive();
    }
};

uint8_t status();
void wait_ready();

void write_enable();
void write_disable();

}

namespace midi_controller {

void spi_eeprom::setup() {
    transfer_scope::setup();
}

void spi_eeprom::get(addr_type addr, uint8_t *b, size_type size) {
    wait_ready();

    {
        transfer_scope ts;

        spi::send(READ);
        spi::send(addr >> 8);
        spi::send(addr & 0xFF);

        for (; size; --size) {
            *b++ = spi::recv();
        }
    }
}

void spi_eeprom::put(addr_type addr, const uint8_t *b, size_type size) {
    wait_ready();
    write_enable();

    {
        transfer_scope ts;

        spi::send(WRITE);
        spi::send(addr >> 8);
        spi::send(addr & 0xFF);

        for (; size; --size) {
            spi::send(*b++);
        }
    }

    write_disable();
}

}

namespace {

uint8_t status() {
    transfer_scope ts;

    spi::send(RDSR);
    return spi::recv();
}

void wait_ready() {
    while ((status() & RDY) != 0) {
        artl::yield();
    }
}

void write_enable() {
    transfer_scope ts;

    spi::send(WREN);
}

void write_disable() {
    transfer_scope ts;

    spi::send(WRDI);
}

}
