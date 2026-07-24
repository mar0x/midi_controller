#pragma once

#include <stdint.h>

#include "settings.h"
#include "profile.h"
#include "program.h"
#include "program_seq.h"
#include "serial_num.h"
#include "hardware_id.h"

namespace midi_controller {

struct storage {
    static void setup();
    static bool check();
    static void reset();

    static void read(settings_t &v);
    static void write(const settings_t &v);
    static void write(const settings_t &s, const uint8_t &v);
    static void write(const settings_t &s, const uint16_t &v);
    static void write_settings(int off, const uint8_t *d, uint8_t size);
    template<typename T>
    static void write(const settings_t &s, const T &v) {
        int off = (const uint8_t *) &v - (const uint8_t *) &s;
        write_settings(off, (const uint8_t *) &v, sizeof(v));
    }

    static void read(uint8_t id, profile_t &v);
    static void write(uint8_t id, const profile_t &v);
    static void write(uint8_t id, const profile_t &f, const uint8_t &v);

    static void read(uint8_t profile_id, uint8_t id, program_t &v);
    static void write(uint8_t profile_id, uint8_t id, const program_t &v);

    static void read(uint8_t profile_id, uint8_t id, program_seq_t &v);
    static uint8_t read_next(uint8_t profile_id, uint8_t id);
    static uint8_t read_prev(uint8_t profile_id, uint8_t id);
    static void write(uint8_t profile_id, uint8_t id, const program_seq_t &v);
    static void write_next(uint8_t profile_id, uint8_t id, uint8_t v);
    static void write_prev(uint8_t profile_id, uint8_t id, uint8_t v);

    static uint8_t program_seq(uint8_t profile_id, uint8_t id);
    static void program_seq(uint8_t profile_id, uint8_t id, uint8_t seq);
    static uint8_t seq_program(uint8_t profile_id, uint8_t seq);

    static void read(serial_num_t &v);
    static void write(const serial_num_t &v);

    static void read(hardware_id_t &v);
    static void write(const hardware_id_t &v);

    static void write_factory_data(uint16_t len);
    static void read_factory_data();
};

}
