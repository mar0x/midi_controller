#pragma once

#define HW0 0

namespace midi_controller {

enum {
#if HW0
    MAX_PROFILE = 2,
    MAX_PROGRAM = 10,
#else
    MAX_PROFILE = 3,
    MAX_PROGRAM = 200,
#endif
};

}
