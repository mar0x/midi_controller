#pragma once

#include "debug.h"
#include "config.h"

namespace midi_controller {

class program_t {
    enum {
        TITLE_SIZE = 12,
    };

public:
    void reset();
    bool empty() const;

    uint8_t title_size() const;

    bool operator==(const program_t& n) const;
    bool operator!=(const program_t& n) const;

    char title[TITLE_SIZE] =
        { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
};

inline void
program_t::reset() {
    for (uint8_t i = 0; i < TITLE_SIZE; ++i) {
        title[i] = ' ';
    }
    loop.reset();
}

inline bool
program_t::empty() const {
    return *this == program_t();
}

inline bool
program_t::operator==(const program_t& n) const {
    return memcmp(title, n.title, sizeof(title)) == 0 &&
      loop == n.loop;
}

inline bool
program_t::operator!=(const program_t& n) const {
    return memcmp(title, n.title, sizeof(title)) != 0 ||
      loop != n.loop;
}

inline uint8_t
program_t::title_size() const {
    for (uint8_t s = TITLE_SIZE; s > 0; --s) {
        if (title[s - 1] != ' ') {
            return s;
        }
    }

    return 0;
}

}
