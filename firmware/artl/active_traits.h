#pragma once

namespace artl {

template<typename PIN_TYPE>
struct active_low {
    using pin_type = PIN_TYPE;

    static void setup() { pin_type::output(); }

    static void active() { pin_type::low(); }
    static void inactive() { pin_type::high(); }
};

template<typename PIN_TYPE>
struct active_pulldown {
    using pin_type = PIN_TYPE;

    static void setup() { }

    static void active() { pin_type::output(); pin_type::pullup(); }
    static void inactive() { pin_type::input(); pin_type::pulldown(); }
};

template<typename PIN_TYPE>
struct active_high {
    using pin_type = PIN_TYPE;

    static void setup() { pin_type::output(); }

    static void active() { pin_type::high(); }
    static void inactive() { pin_type::low(); }
};

struct active_dummy {
    static void setup() { }

    static void active() { }
    static void inactive() { }
};

}
