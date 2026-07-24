#pragma once

#include <stdint.h>

namespace midi_controller {

struct settings_v0_t {
    uint8_t size;
    uint8_t version;
    uint8_t midi_channel_in;
    uint8_t midi_out_prog;
    uint8_t midi_fwd;
    uint8_t usb_debug;
    uint8_t hide_cursor_delay_s;
    uint16_t midi_send_delay_ms;
    uint16_t store_program_delay_ms;
    uint8_t program_start;
    uint8_t channel_start;
    uint8_t active_profile;
    uint8_t echo;
    char title[17];

    void reset();
};

using settings_t = settings_v0_t;

extern settings_t settings;

}
