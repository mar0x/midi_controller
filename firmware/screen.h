#pragma once

#include "screen_buf.h"
#include "display_1602.h"

namespace midi_controller {

using screen = screen_buf_t<display_1602>;

}
