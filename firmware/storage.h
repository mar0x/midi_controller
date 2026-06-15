
#pragma once

#include "config.h"
#include "settings.h"
#include "program.h"
#include "serial_num.h"
#include "hardware_id.h"

namespace patch_mate {

static const char eemagic[4] = { 'M', 'P', 'M', '5' };

struct storage {

    enum { SETTINGS_START = sizeof(eemagic) };

    using settings_storage = eeprom;

#if defined(HAVE_SPI_EEPROM)
    enum { PROGRAM_START = 16 };

    using program_storage = spi_eeprom;
#else
    enum { PROGRAM_START = SETTINGS_START + sizeof(settings_t) };

    using program_storage = eeprom;
#endif

    static void setup() {
        settings_storage::setup();
        program_storage::setup();
    }

    static bool check() {
        char b[sizeof(eemagic)];

        settings_storage::get(0, b);
        if (memcmp(eemagic, b, sizeof(eemagic)) != 0) {
            return false;
        }

        program_storage::get(0, b);
        if (memcmp(eemagic, b, sizeof(eemagic)) != 0) {
            return false;
        }

        return true;
    }

    static void reset() {
        settings_storage::put(0, eemagic);

        write(settings_t());

        program_storage::put(0, eemagic);

        program_t z;
        for (uint8_t i = 0; i < MAX_PROGRAMS; i++) {
            write(i, z);
        }
    }

    static void read(uint8_t id, program_t &v) {
        debug(7, "program::read: ", id);

        program_storage::get(PROGRAM_START + sizeof(program_t) * id, v);
    }

    static void write(uint8_t id, const program_t &v) {
        debug(7, "program::write: ", id);

        program_storage::put(PROGRAM_START + sizeof(program_t) * id, v);
    }

    static void read(settings_t &v) {
        settings_storage::get(SETTINGS_START, v);
    }

    static void write(const settings_t &v) {
        settings_storage::put(SETTINGS_START, v);
    }

    static void write(const settings_t &s, const uint8_t &v) {
        int off = &v - (const uint8_t *) &s;
        settings_storage::put(SETTINGS_START + off, v);
    }

    static void write(const settings_t &s, const uint16_t &v) {
        int off = (const uint8_t *) &v - (const uint8_t *) &s;
        settings_storage::put(SETTINGS_START + off, v);
    }

    static void read(serial_num_t &v) {
        eeprom::get(eeprom::length() - sizeof(v), v);
    }

    static void write(const serial_num_t &v) {
        eeprom::put(eeprom::length() - sizeof(v), v);

#if defined(HAVE_SPI_EEPROM)
        spi_eeprom::put(spi_eeprom::length() - sizeof(v), v);
#endif
    }

    static void read(hardware_id_t &v) {
        eeprom::get(eeprom::length() - sizeof(v) - sizeof(serial_num_t), v);
    }

    static void write(const hardware_id_t &v) {
        eeprom::put(eeprom::length() - sizeof(v) - sizeof(serial_num_t), v);

#if defined(HAVE_SPI_EEPROM)
        spi_eeprom::put(spi_eeprom::length() - sizeof(v) - sizeof(serial_num_t), v);
#endif
    }

};

}
