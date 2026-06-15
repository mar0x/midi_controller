
#pragma once

#include <stdint.h>

namespace artl {

template<typename CLOCK>
struct debouncer_state {
    using clock_type = typename CLOCK::value_type;

    bool state;
    bool raw_state;
    clock_type changed_at;

    void setup(bool v) {
        state = raw_state = v;
        changed_at = CLOCK::get();
    }

    bool update(bool v, clock_type period_high, clock_type period_low) {
        if (raw_state != v) {
            raw_state = v;
            changed_at = CLOCK::get();
        }

        if (state != raw_state &&
            (raw_state ?
                CLOCK::timeout(changed_at, period_high) :
                CLOCK::timeout(changed_at, period_low))) {
            state = raw_state;
            return true;
        }

        return false;
    }
};

template<typename BASE_IN, typename CLOCK,
    int BOUNCE_PERIOD, int BOUNCE_LOW_PERIOD = BOUNCE_PERIOD>
struct debounced_in {
    using state_type = debouncer_state<CLOCK>;
    using clock_type = typename CLOCK::value_type;

    static state_type state;

    static void setup() {
        BASE_IN::setup();

        state.setup(BASE_IN::read());
    }

    static bool read() { return state.state; }

    static bool update() {
        bool v = BASE_IN::read();

        return state.update(v, BOUNCE_PERIOD, BOUNCE_LOW_PERIOD);
    }
};

template<typename BASE_IN, typename CLOCK, int BOUNCE_PERIOD, int BOUNCE_LOW_PERIOD>
debouncer_state<CLOCK> debounced_in<BASE_IN, CLOCK, BOUNCE_PERIOD, BOUNCE_LOW_PERIOD>::state;

}
