#pragma once

#include "artl/static_clock.h"
#include "artl/stimer.h"

namespace midi_controller {

enum {
    STIMER_NONE,
    STIMER_CLOCK,
    STIMER_DELAY_OK,
    STIMER_HOLD_OK,
    STIMER_DELAY_UP,
    STIMER_DELAY_DOWN,
    STIMER_DELAY_LEFT,
    STIMER_DELAY_RIGHT,
    STIMER_DELAY_ENC_A,
    STIMER_DELAY_ENC_B,
    STIMER_HIDE_CURSOR,
    STIMER_SCROLL_TITLE,
    STIMER_DISPLAY_UPDATE,
    STIMER_OK_BLINK,
    STIMER_UP_BLINK,
    STIMER_DOWN_BLINK,
    STIMER_DELAY_MIDI_PROGRAM,
    STIMER_DELAY_STORE_PROGRAM,
    STIMER_MAX,
};

enum {
    BTN_DELAY_MS = 5,
    BTN_HOLD_MS = 1000,

    FR_OK_BLINK_DELAY_MS = 200,
    BLINK_DELAY_MS = 200,

    SCROLL_TITLE_START_DELAY_MS = 1000,
    SCROLL_TITLE_MOVE_DELAY_MS = 500,
};

using clk_t = artl::clock_traits<uint16_t>;
using stimer = artl::stimer<STIMER_MAX, STIMER_NONE, clk_t>;

}
