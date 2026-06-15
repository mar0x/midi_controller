
#pragma once

#include <stdint.h>

namespace artl {

template<uint8_t N, uint8_t NONE, typename CLOCK>
struct stimer {
    using callback_t = void (*)(uint8_t id);
    using clock = CLOCK;
    using clock_type = typename CLOCK::value_type;

    struct list_type {
        clock_type at;
        clock_type period;
        callback_t callback;
        uint8_t next;
        uint8_t prev;
    };

    // Get next ready timer or NONE.
    static uint8_t get() {
        uint8_t i = list[NONE].next;

        while (i != NONE && clock::timeout(list[i].at)) {
            remove(i);
            if (list[i].period) {
                schedule_in(i, list[i].period, list[i].period);
            }
            if (list[i].callback == nullptr) {
                return i;
            }
            list[i].callback(i);
            i = list[NONE].next;
        }

        return NONE;
    }

    static void update() {
        for(uint8_t i = get(); i != NONE; i = get()) { }
    }

    static void schedule(uint8_t id, clock_type at) {
        uint8_t i;

        cancel(id);

        for (i = list[NONE].next; i != NONE; i = list[i].next) {
            if (CLOCK::gt(list[i].at, at)) {
                break;
            }
        }

        insert_before(id, i);
        list[id].at = at;
        list[id].period = 0;
    }

    static void schedule(uint8_t id, clock_type at, clock_type period) {
        schedule(id, at);
        list[id].period = period;
    }

    static void schedule(uint8_t id, clock_type at, clock_type period, callback_t cb) {
        schedule(id, at, period);
        callback(id, cb);
    }

    static void schedule(uint8_t id, clock_type at, callback_t cb) {
        schedule(id, at);
        callback(id, cb);
    }

    static void schedule_in(uint8_t id, clock_type wait) {
        schedule(id, CLOCK::get() + wait);
    }

    static void schedule_in(uint8_t id, clock_type wait, clock_type period) {
        schedule(id, CLOCK::get() + wait, period);
    }

    static void schedule_in(uint8_t id, clock_type wait, clock_type period, callback_t cb) {
        schedule(id, CLOCK::get() + wait, period);
        callback(id, cb);
    }

    static void schedule_in(uint8_t id, clock_type wait, callback_t cb) {
        schedule(id, CLOCK::get() + wait);
        callback(id, cb);
    }

    static bool cancel(uint8_t id) {
        if (active(id)) {
            remove(id);
            return true;
        }

        return false;
    }

    static bool active(uint8_t id) {
        return list[id].next != id;
    }

    static clock_type at(uint8_t id) {
        return list[id].at;
    }

    static void setup() {
        for (uint8_t i = 0; i < N; ++i) {
            list[i].next = i;
            list[i].callback = nullptr;
        }
        list[NONE].next = NONE;
        list[NONE].prev = NONE;
    }

    static void callback(uint8_t id, callback_t cb) {
        list[id].callback = cb;
    }

private:

    static void insert_before(uint8_t id, uint8_t i) {
        uint8_t prev = list[i].prev;
        list[id].next = i;
        list[id].prev = prev;
        list[i].prev = id;
        list[prev].next = id;
    }

    static void remove(uint8_t id) {
        uint8_t next = list[id].next;
        uint8_t prev = list[id].prev;

        list[next].prev = prev;
        list[prev].next = next;

        list[id].next = id;
    }

    static list_type list[N];
};

template<uint8_t N, uint8_t NONE, typename CLOCK>
typename stimer<N, NONE, CLOCK>::list_type stimer<N, NONE, CLOCK>::list[N];

}
