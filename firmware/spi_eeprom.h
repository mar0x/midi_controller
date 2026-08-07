#pragma once

#include <stdint.h>

namespace midi_controller {

struct spi_eeprom {
    using addr_type = uint16_t;
    using size_type = uint16_t;

    static void setup();
    static bool ready();
    static constexpr size_type length() { return 0x8000; }

    static void get(addr_type addr, uint8_t *b, size_type size);
    static void put(addr_type addr, const uint8_t *b, size_type size);

    template<typename T>
    static void get(addr_type addr, T &t) {
        uint8_t *b = (uint8_t*) &t;
        get(addr, b, sizeof(t));
    }

    template<typename T>
    static void put(addr_type addr, const T &t) {
        const uint8_t *b = (const uint8_t*) &t;
        put(addr, b, sizeof(t));
    }
};

}
