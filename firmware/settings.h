#pragma once

#include <stdint.h>

namespace midi_controller {

struct settings_v0_t {
    uint8_t size = sizeof(settings_v0_t);
    uint8_t version = 0;
    uint8_t midi_channel_in = 0;
    uint8_t midi_channel_out = 0;
    uint8_t midi_out_prog = 0;
    uint8_t midi_fwd = 0;
    uint8_t usb_debug = 0;
    uint16_t mute_delay_ms = 50;
    uint8_t hide_cursor_delay_s = 5;
    uint8_t prog_start = 1;
    uint8_t chan_start = 1;
    uint8_t echo = 0;
};

using settings_t = settings_v0_t;

}
