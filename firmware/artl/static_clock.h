
#pragma once

#include <stdint.h>
#include <Arduino.h>

namespace artl {

template<typename T>
struct static_clock_store {
    using value_type = T;

    static value_type t;

    inline static void set(value_type v) { t = v; }

    inline static value_type get() { return t; }

    static void update() { set(millis()); }
};

template<typename T>
T static_clock_store<T>::t;

template<typename T = uint32_t, typename SRC = static_clock_store<uint32_t> >
struct clock_traits {
    using value_type = T;
    using clock_source = SRC;

    static constexpr value_type max_value() { return ((value_type) -1); }
    static constexpr value_type max_delta() { return max_value() / 2; }

    static value_type get() { return clock_source::get(); }
    static void update() { clock_source::update(); }

    static bool timeout(value_type start, value_type period) { return get() - start >= period; }

    static bool timeout(value_type v) { return ge(get(), v); }

    static bool gt(value_type a, value_type b) { return a - 1 - b < max_delta(); }

    static bool ge(value_type a, value_type b) { return a - b < max_delta();  }
};

}
