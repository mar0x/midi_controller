#pragma once

#include "debug.h"

namespace midi_controller {

template<typename T> T rotate(T v, unsigned int max, int dir) {
    T res = (v + max + dir) % max;
    debug(2, "rotate: ", v, ", ", max, ", ", dir, " = ", res);
    return res;
}

template<typename T> T rotate(T v, unsigned int min, unsigned int max, int dir) {
    unsigned int range = max - min;
    T res = min + (v - min + range + dir) % range;
    debug(2, "rotate: ", v, ", ", min, ", ", max, ", ", dir, " = ", res);
    return res;
}

}
