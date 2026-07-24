#pragma once

#include "title.h"

namespace midi_controller {

enum {
    PROFILE_MOBIUS = 0,
    PROFILE_ROLAND_BD1 = 1,
    PROFILE_ADA_MP1 = 2,
    PROFILE_CUSTOM = 3,
};

struct profile_t : public title_t<27> {

    void reset();

    uint8_t channel;
    uint8_t port_mask;
    uint8_t active_program;
};

}
