#pragma once

#include <stdint.h>

namespace midi_controller {

struct serial_num_t {
    char buf[sizeof("NNNN")];

    bool empty() const { return buf[0] == 0 || ((uint8_t) buf[0]) == 0xFFu; }

    operator char*() { return buf; }
    operator const char*() const { return buf; }
};

}
