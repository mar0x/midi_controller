#pragma once

namespace artl {

struct delayed_in_base {
    static void setup();
    static bool read();
};

struct delayed_in_scheduler {
    using callback_t = void (*)();

    static void schedule(bool rise, callback_t cb);
    static void cancel();
};

struct delayed_in_edge_cb {
    static void rise();
    static void fall();
};

template<
    typename STIMER,
    uint8_t ID,
    typename STIMER::clock_type RISE_DELAY,
    typename STIMER::clock_type FALL_DELAY = RISE_DELAY>
struct stimer_scheduler {
    using callback_t = void (*)();

    static void schedule(bool rise, callback_t cb) {
        STIMER::schedule_in(ID, rise ? RISE_DELAY : FALL_DELAY, cb);
    }

    static void cancel() {
        STIMER::cancel(ID);
    }
};

template<typename BASE_IN, typename SCHEDULER, typename EDGE_CB>
struct delayed_in {
    static bool state;
    static bool raw_state;

    static void setup() {
        BASE_IN::setup();
        state = raw_state = BASE_IN::read();
    }

    static bool read() { return state; }

    static void delayed() {
        if (state != raw_state) {
            state = raw_state;
            if (state) {
                EDGE_CB::rise();
            } else {
                EDGE_CB::fall();
            }
        }
    }

    static void update() {
        bool v = BASE_IN::read();

        if (raw_state == v) {
            return;
        }

        raw_state = v;
        if (state != v) {
            SCHEDULER::schedule(v, delayed);
        } else {
            SCHEDULER::cancel();
        }
    }
};

template<typename BASE_IN, typename SCHEDULER, typename EDGE_CB>
bool delayed_in<BASE_IN, SCHEDULER, EDGE_CB>::state;

template<typename BASE_IN, typename SCHEDULER, typename EDGE_CB>
bool delayed_in<BASE_IN, SCHEDULER, EDGE_CB>::raw_state;

}
