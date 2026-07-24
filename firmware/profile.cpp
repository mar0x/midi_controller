#include "config.h"
#include "profile.h"
#include <string.h>

namespace midi_controller {

void profile_t::reset() {

    title_t::reset();

    channel = 0;
    port_mask = 3;
    active_program = 0;
}

}
