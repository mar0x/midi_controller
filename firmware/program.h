
#pragma once

#include "debug.h"
#include "config.h"

namespace patch_mate {

class program_t {
    enum {
        TITLE_SIZE = 12,
    };

public:
    class loop_t {
    public:
        loop_t(loop_value_t v = 0) : value_(v) { }

        void set(uint8_t i, bool s);
        bool toggle(uint8_t i);

        bool operator[](uint8_t i) const;
        operator loop_value_t() const { return value_; }
        bool operator==(const loop_t& s) const { return value_ == s.value_; }
        bool operator!=(const loop_t& s) const { return value_ != s.value_; }

        void reset() { value_ = 0; }

    private:
        loop_value_t value_ = 0;
    };

    void reset();
    bool empty() const;

    uint8_t title_size() const;

    bool operator==(const program_t& n) const;
    bool operator!=(const program_t& n) const;

    char title[TITLE_SIZE] =
        { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
    loop_value_t zero = 0;
    loop_t loop;
};

inline void
program_t::loop_t::set(uint8_t i, bool s) {
    if (s) {
        value_ |= (1 << i);
    } else {
        value_ &= ~(1 << i);
    }
}

inline bool
program_t::loop_t::toggle(uint8_t i) {
    uint8_t m = (1 << i);

    return (value_ ^= m) & m;
}

inline bool
program_t::loop_t::operator[](uint8_t i) const {
    return (value_ & (1 << i)) != 0;
}

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
