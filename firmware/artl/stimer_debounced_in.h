#pragma once

namespace artl {

template<typename BASE_IN, typename DELAY_TRAITS, typename EDGE_CB>
struct stimer_debounced_in {
    static bool state;
    static bool raw_state;

    static void setup() {
        BASE_IN::setup();
        state = raw_state = BASE_IN::read();
    }

    static bool read() { return state; }

    static void debounced(uint8_t id) {
        if (state != raw_state) {
            state = raw_state;
            if (state) {
                EDGE_CB::rise(id);
            } else {
                EDGE_CB::fall(id);
            }
        }
    }

    static void update(uint8_t id) {
        bool v = BASE_IN::read();

        if (raw_state == v) {
            return;
        }

        raw_state = v;
        if (state != v) {
            DELAY_TRAITS::schedule(id, v, debounced);
        } else {
            DELAY_TRAITS::cancel(id);
        }
    }
};

template<typename BASE_IN, typename DELAY_TRAITS, typename EDGE_CB>
bool stimer_debounced_in<BASE_IN, DELAY_TRAITS, EDGE_CB>::state;

template<typename BASE_IN, typename DELAY_TRAITS, typename EDGE_CB>
bool stimer_debounced_in<BASE_IN, DELAY_TRAITS, EDGE_CB>::raw_state;

}
