#include "storage.h"

#include <string.h>
#include <stddef.h>
#include "eeprom.h"
#include "spi_eeprom.h"
#include "config.h"
#include <Arduino.h>
#include "serial_cmd.h"
#include "current_state.h"
#include "led.h"

namespace {

const char eemagic[4] = { 'M', 'C', 'T', '1' };

}

namespace midi_controller {

enum { SETTINGS_START = sizeof(eemagic) };

using settings_storage = eeprom;

enum {
    // PROFILE_START = 32,
    PROFILE_START = SETTINGS_START + sizeof(settings_t) + 32,
    PROGRAM_START = PROFILE_START + MAX_PROFILE * (sizeof(profile_t) + sizeof(program_seq_t)),
};

//using profile_storage = spi_eeprom;
using profile_storage = eeprom;

using program_storage = profile_storage;
using factory_storage = profile_storage;

namespace {

uint16_t get_profile_addr(uint8_t profile_id) {
    return PROFILE_START +
        (sizeof(profile_t) + sizeof(program_seq_t)) * profile_id;
}

uint16_t get_program_addr(uint8_t profile_id, uint8_t program_id) {
    return PROGRAM_START +
        (sizeof(program_t) + sizeof(program_seq_t)) *
            (MAX_PROGRAM * profile_id + program_id);
}

uint16_t get_program_seq_addr(uint8_t profile_id, uint8_t program_id) {
    return (program_id != MAX_PROGRAM) ?
                PROGRAM_START +
                (sizeof(program_t) + sizeof(program_seq_t)) *
                    (MAX_PROGRAM * profile_id + program_id) +
                sizeof(program_t)
          :
                PROFILE_START +
                (sizeof(profile_t) + sizeof(program_seq_t)) * profile_id +
                sizeof(profile_t);
}

uint16_t get_factory_data_addr() {
    return PROGRAM_START +
        (sizeof(program_t) + sizeof(program_seq_t)) *
            (MAX_PROGRAM * MAX_PROFILE);
}

}

void storage::setup() {
    eeprom::setup();
    spi_eeprom::setup();
}

bool storage::check() {
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

void storage::reset() {
    settings_storage::put(0, eemagic);

    settings_t settings;
    settings.reset();
    write(settings);
/*
    program_storage::put(0, eemagic);
*/

    profile_t f;
    f.reset();

    program_t p;
    p.reset();

    program_seq_t s;

    for (uint8_t profile_id = 0; profile_id < MAX_PROFILE; ++profile_id) {
        switch (profile_id) {
        case PROFILE_MOBIUS:
            f.title("Mobius", 6);
            break;
        case PROFILE_ROLAND_BD1:
            f.title("Roland BD1", 10);
            break;
        case PROFILE_ADA_MP1:
            f.title("ADA MP1", 7);
            break;
        case PROFILE_CUSTOM:
            f.title("Custom", 6);
            break;
        }

        write(profile_id, f);

        s.next = 0;
        s.prev = MAX_PROGRAM - 1;
        write(profile_id, MAX_PROGRAM, s);

        for (uint8_t i = 0; i < MAX_PROGRAM; ++i) {
            write(profile_id, i, p);

            s.next = i + 1;
            s.prev = i > 0 ? i - 1 : MAX_PROGRAM;
            write(profile_id, i, s);
        }
    }

/*
    uint16_t addr = get_factory_data_addr();
    uint16_t len;

    factory_storage::get(addr, len);

    addr += sizeof(len);

    serial_cmd_t cmd;

    while (len) {
        uint8_t c;
        factory_storage::get(addr, c);
        cmd.read(c);

        if (cmd) {
            process_cmd(cmd, false);

            cmd.reset();
        }

        addr += 1;
        len -= 1;
    }
*/
}

void storage::read(settings_t &v) {
    settings_storage::get(SETTINGS_START, v);
}

void storage::write(const settings_t &v) {
    settings_storage::put(SETTINGS_START, v);
}

void storage::write(const settings_t &s, const uint8_t &v) {
    int off = &v - (const uint8_t *) &s;
    settings_storage::put(SETTINGS_START + off, v);
}

void storage::write(const settings_t &s, const uint16_t &v) {
    int off = (const uint8_t *) &v - (const uint8_t *) &s;
    settings_storage::put(SETTINGS_START + off, v);
}

void storage::write_settings(int off, const uint8_t *d, uint8_t size) {
    settings_storage::put(SETTINGS_START + off, d, size);
}

void storage::read(uint8_t id, profile_t &v) {
    uint16_t addr = get_profile_addr(id);
    profile_storage::get(addr, v);
}

void storage::write(uint8_t id, const profile_t &v) {
    uint16_t addr = get_profile_addr(id);
    profile_storage::put(addr, v);
}

void storage::write(uint8_t id, const profile_t &f, const uint8_t &v) {
    uint16_t addr = get_profile_addr(id);
    int off = &v - (const uint8_t *) &f;
    profile_storage::put(addr + off, v);
}

void storage::read(uint8_t profile_id, uint8_t id, program_t &v) {
    uint16_t addr = get_program_addr(profile_id, id);
    program_storage::get(addr, v);
}

void storage::write(uint8_t profile_id, uint8_t id, const program_t &v) {
    uint16_t addr = get_program_addr(profile_id, id);
    program_storage::put(addr, v);
}

void storage::read(uint8_t profile_id, uint8_t id, program_seq_t &v) {
    uint16_t addr = get_program_seq_addr(profile_id, id);
    program_storage::get(addr, v);
}

uint8_t storage::read_next(uint8_t profile_id, uint8_t id) {
    uint16_t addr = get_program_seq_addr(profile_id, id) +
        offsetof(program_seq_t, next);
    uint8_t v;
    program_storage::get(addr, v);
    return v;
}

uint8_t storage::read_prev(uint8_t profile_id, uint8_t id) {
    uint16_t addr = get_program_seq_addr(profile_id, id) +
        offsetof(program_seq_t, prev);
    uint8_t v;
    program_storage::get(addr, v);
    return v;
}

void storage::write(uint8_t profile_id, uint8_t id, const program_seq_t &v) {
    uint16_t addr = get_program_seq_addr(profile_id, id);
    program_storage::put(addr, v);
}

void storage::write_next(uint8_t profile_id, uint8_t id, uint8_t v) {
    uint16_t addr = get_program_seq_addr(profile_id, id) +
        offsetof(program_seq_t, next);
    program_storage::put(addr, v);
}

void storage::write_prev(uint8_t profile_id, uint8_t id, uint8_t v) {
    uint16_t addr = get_program_seq_addr(profile_id, id) +
        offsetof(program_seq_t, prev);
    program_storage::put(addr, v);
}

uint8_t storage::program_seq(uint8_t profile_id, uint8_t id) {
    for (uint8_t i = 0; i < MAX_PROGRAM; ++i) {
        id = read_prev(profile_id, id);
        if (id == MAX_PROGRAM) return i;
    }

    return MAX_PROGRAM;
}

void storage::program_seq(uint8_t profile_id, uint8_t id, uint8_t seq) {
    uint8_t pseq = program_seq(profile_id, id);
    if (pseq == seq) return;

    program_seq_t s;
    read(profile_id, id, s);

    uint8_t pnext = s.next;
    uint8_t pprev = s.prev;

    // 1. remove 'id' from sequence
    write_prev(profile_id, pnext, pprev);
    write_next(profile_id, pprev, pnext);

    // 2. find program on 'seq' place
    pnext = seq_program(profile_id, seq);

    // 3. insert before
    pprev = read_prev(profile_id, pnext);
    write_prev(profile_id, pnext, id);
    write_next(profile_id, pprev, id);

    s.prev = pprev;
    s.next = pnext;
    write(profile_id, id, s);
}

uint8_t storage::seq_program(uint8_t profile_id, uint8_t seq) {
    uint8_t p = MAX_PROGRAM;

    while (true) {
        p = read_next(profile_id, p);
        if (seq == 0) return p;

        --seq;
    }
}

void storage::read(serial_num_t &v) {
    eeprom::get(eeprom::length() - sizeof(v), v);
}

void storage::write(const serial_num_t &v) {
    eeprom::put(eeprom::length() - sizeof(v), v);

    spi_eeprom::put(spi_eeprom::length() - sizeof(v), v);
}

void storage::read(hardware_id_t &v) {
    eeprom::get(eeprom::length() - sizeof(v) - sizeof(serial_num_t), v);
}

void storage::write(const hardware_id_t &v) {
    eeprom::put(eeprom::length() - sizeof(v) - sizeof(serial_num_t), v);

    spi_eeprom::put(spi_eeprom::length() - sizeof(v) - sizeof(serial_num_t), v);
}

void storage::write_factory_data(uint16_t len) {
    uint16_t addr = get_factory_data_addr();
    uint8_t buf[32];
    uint8_t pos = sizeof(uint16_t);

    ((uint16_t *)buf)[0] = len;

    while (len && Serial.dtr()) {
        if (Serial.available()) {
            uint8_t c = Serial.read();
            buf[pos] = c;
            ++pos;

            if (pos >= 32) {
                spi_eeprom::put(addr, buf, pos);

                addr += pos;
                pos = 0;

                led::toggle_ok();
            }
            len -= 1;
        }
    }

    if (pos > 0) {
        spi_eeprom::put(addr, buf, pos);

        addr += pos;
        pos = 0;
    }

    led::ok(false);
}

void storage::read_factory_data() {
    uint16_t addr = get_factory_data_addr();
    uint16_t len;

    spi_eeprom::get(addr, len);

    addr += sizeof(len);

    while (len && Serial.dtr()) {
        uint8_t c;
        spi_eeprom::get(addr, c);
        Serial.write(c);

        addr += 1;
        len -= 1;
    }
}

}
