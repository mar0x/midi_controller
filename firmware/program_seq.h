#pragma once

#include <stdint.h>

namespace midi_controller {

struct program_seq_t {
    uint8_t prev;
    uint8_t next;
};

}
