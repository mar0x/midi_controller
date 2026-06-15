#pragma once

#include "config.h"
#include "debug.h"

namespace patch_mate {

struct settings_t {
    uint8_t midi_channel = 0;
    uint8_t midi_loop_ctrl_in[10] = { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 };
    uint8_t midi_loop_ctrl_out[10] = { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 };
    uint8_t midi_out_prog = 0;
    uint8_t midi_fwd = 0;
    uint8_t usb_debug = 0;
    uint16_t mute_delay_ms = 50;
    uint8_t hide_cursor_delay_s = 5;
    uint8_t prog_start = 1;
    uint8_t chan_start = 1;
    uint8_t echo = 0;
    uint8_t not_used_yet[2] = {0, 0};
};

}
