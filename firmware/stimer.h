#pragma once

#include "artl/static_clock.h"
#include "artl/stimer.h"

namespace midi_controller {

enum {
    STIMER_NONE,
    STIMER_CLOCK,
    STIMER_DELAY_OK,
    STIMER_DELAY_UP,
    STIMER_DELAY_DOWN,
    STIMER_DELAY_LEFT,
    STIMER_DELAY_RIGHT,
    STIMER_DELAY_ENC_A,
    STIMER_DELAY_ENC_B,
    STIMER_HIDE_CURSOR,
    STIMER_SCROLL_TITLE,
    STIMER_MAX,
};

using clk_t = artl::clock_traits<uint16_t>;
using stimer = artl::stimer<STIMER_MAX, STIMER_NONE, clk_t>;

}
