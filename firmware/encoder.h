#pragma once

#include <stdint.h>

namespace midi_controller {

struct encoder {
    static void setup();
    static void update();

    static void on_rotate(int8_t d);
};

}
