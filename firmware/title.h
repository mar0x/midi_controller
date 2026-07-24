#pragma once

#include <stdint.h>
#include <string.h>

namespace midi_controller {

template<uint8_t SIZE>
struct title_t {
    enum {
        DATA_SIZE = SIZE,
    };

    void reset() {
        memset(data, 0, DATA_SIZE);
    }

    uint8_t title_size() const { return strlen(title()); }
    const char * title() const { return (const char *) data; }
    void title(const char *t, uint8_t size) {
        if (size >= DATA_SIZE) {
            size = DATA_SIZE - 1;
        }
        memcpy(data, t, size);
        memset(data + size, 0, DATA_SIZE - size);
    }

    uint8_t data[DATA_SIZE];
};

}
