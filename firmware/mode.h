#pragma once

#include <stdint.h>

namespace midi_controller {

enum {
    MODE_INIT,
    MODE_PROGRAM_SELECT,
    MODE_PROFILE_SELECT,
    MODE_SETTINGS_FIRST,
    MODE_SETTINGS_CHANNEL = MODE_SETTINGS_FIRST,
    MODE_SETTINGS_PROGRAM_START,
    MODE_SETTINGS_CHANNEL_START,
    MODE_SETTINGS_FACTORY_RESET,
    MODE_MAX,

    MODE_SETTINGS_PROG_OUT,
    MODE_SETTINGS_MIDI_FWD,
    MODE_SETTINGS_HIDE_CURSOR,
    MODE_SETTINGS_USB_DEBUG,
    MODE_SETTINGS_LAST = MODE_SETTINGS_FACTORY_RESET,
    MODE_PROGRAM_SWAP,
    MODE_PROGRAM_MOVE,
    MODE_MIDI_IN_MONITOR,
    MODE_MIDI_OUT_MONITOR,
    MODE_MIDI_DUMP_SEND,
    MODE_MIDI_DUMP_RECV,
    MODE_ABOUT,
    MODE_UPTIME,
};

struct mode_t {
    virtual void enter() const { }
    virtual void leave() const { }
    virtual void on_rotate(int8_t) const { }
    virtual void on_ok_press() const { }
    virtual void on_ok_hold() const { }
};

extern uint8_t cur_mode;

void mode_init();
void set_mode(uint8_t m);
const mode_t *current_mode();

}
