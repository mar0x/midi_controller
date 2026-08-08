#pragma once

#include <stdint.h>

namespace midi_controller {

struct hardware_id_t {
    char buf[sizeof("YYYY.MM.DD")];

    bool empty() const { return buf[0] == 0 || ((uint8_t) buf[0]) == 0xFFu; }

    operator char*() { return buf; }
    operator const char*() const { return buf; }
};

}
