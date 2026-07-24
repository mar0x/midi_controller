#include "settings.h"

#include <avr/pgmspace.h>

namespace midi_controller {

namespace {
    const char default_title[] PROGMEM = "MIDI Controller";
}

void settings_v0_t::reset() {
    size = sizeof(settings_v0_t);
    version = 0;
    midi_channel_in = 0;
    midi_out_prog = 0;
    midi_fwd = 0;
    usb_debug = 0;
    hide_cursor_delay_s = 5;
    midi_send_delay_ms = 50;
    store_program_delay_ms = 1000;
    program_start = 1;
    channel_start = 1;
    active_profile = 0;
    echo = 0;
    strncpy_P(title, default_title, sizeof(title) - 1);
}

settings_t settings;

}
