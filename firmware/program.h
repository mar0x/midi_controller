#pragma once

#include <stdint.h>
#include <string.h>

namespace midi_controller {

class program_t {
    enum {
        TITLE_SIZE = 12,
        DATA_SIZE = 31,
    };

public:
    void reset();
    bool empty() const;

    uint8_t title_size() const;
    const char * title() const;
    void title(const char *t, uint8_t size);

    bool operator==(const program_t& n) const;
    bool operator!=(const program_t& n) const;

    uint8_t type;
    uint8_t data[DATA_SIZE];
};

inline void
program_t::reset() {
    type = 0;
    memset(data, 0, DATA_SIZE);
}

inline bool
program_t::empty() const {
    return *this == program_t();
}

inline bool
program_t::operator==(const program_t& n) const {
    return type == n.type && memcmp(data, n.data, sizeof(data)) == 0;
}

inline bool
program_t::operator!=(const program_t& n) const {
    return type != n.type || memcmp(data, n.data, sizeof(data)) != 0;
}

inline const char *
program_t::title() const {
    return (const char *) data;
}

inline void
program_t::title(const char *t, uint8_t size) {
    memcpy(data, t, size);
    data[size] = 0;
}

inline uint8_t
program_t::title_size() const {
    return strlen(title());
/*    return TITLE_SIZE;

    for (uint8_t s = TITLE_SIZE; s > 0; --s) {
        if (title[s - 1] != ' ') {
            return s;
        }
    }

    return 0;
*/
}

}
