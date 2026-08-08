#pragma once

namespace midi_controller {

template <typename T>
struct prog_var_t {
    prog_var_t(const T& v): ref(v) { }

    const T& get(T& out) const {
        memcpy_P(&out, &ref, sizeof(T));
        return out;
    }

    const T& ref;
};

}
