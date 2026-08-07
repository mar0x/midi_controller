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

enum {
    PAGE_SIZE = 64,
    PAGE_MASK = 0xFFC0,
};

using midi_controller::eeprom_cs;
using midi_controller::spi;
using addr_type = midi_controller::spi_eeprom::addr_type;

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

void get_page(addr_type addr, uint8_t *b, uint8_t size);
void put_page(addr_type addr, const uint8_t *b, uint8_t size);

}

namespace midi_controller {

void spi_eeprom::setup() {
    transfer_scope::setup();
}

bool spi_eeprom::ready() {
    return (status() & RDY) == 0;
}

void spi_eeprom::get(addr_type addr, uint8_t *b, size_type size) {
    while (size) {
        addr_type chunk_page = addr & PAGE_MASK;
        size_type chunk_size = size;
        size_type max_chunk_size = chunk_page + PAGE_SIZE - addr;

        if (chunk_size > max_chunk_size) {
            chunk_size = max_chunk_size;
        }

        get_page(addr, b, chunk_size);

        addr += chunk_size;
        size -= chunk_size;
        b += chunk_size;
    }
}

void spi_eeprom::put(addr_type addr, const uint8_t *b, size_type size) {
    while (size) {
        addr_type chunk_page = addr & PAGE_MASK;
        size_type chunk_size = size;
        size_type max_chunk_size = chunk_page + PAGE_SIZE - addr;

        if (chunk_size > max_chunk_size) {
            chunk_size = max_chunk_size;
        }

        put_page(addr, b, chunk_size);

        addr += chunk_size;
        size -= chunk_size;
        b += chunk_size;
    }
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

void get_page(addr_type addr, uint8_t *b, uint8_t size) {
    // size <= PAGE_SIZE
    // (addr & PAGE_MASK) == ((addr + size - 1) & PAGE_MASK)

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

void put_page(addr_type addr, const uint8_t *b, uint8_t size) {
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
