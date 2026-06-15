
#pragma once

#include <stdint.h>

namespace artl {

template<uint8_t N, uint8_t NONE, typename V>
struct slist {
    using value_type = V;

    void setup() {
        for (uint8_t i = 0; i < N; ++i) {
            list[i].next = i;
        }
        list[NONE].next = NONE;
        list[NONE].prev = NONE;
    }

    uint8_t pop_front() {
        uint8_t i = list[NONE].next;

        if (i != NONE) {
            remove(i);
        }

        return i;
    }

    void push_front(uint8_t id) {
        cancel(id);

        insert_before(id, list[NONE].next);
    }

    void push_back(uint8_t id) {
        cancel(id);

        insert_before(id, NONE);
    }

    bool active(uint8_t id) const {
        return list[id].next != id;
    }

    bool cancel(uint8_t id) {
        if (active(id)) {
            remove(id);
            return true;
        }

        return false;
    }

    value_type at(uint8_t id) const {
        return list[id].v;
    }

private:
    void insert_before(uint8_t id, uint8_t i) {
        uint8_t prev = list[i].prev;
        list[id].next = i;
        list[id].prev = prev;
        list[i].prev = id;
        list[prev].next = id;
    }

    void remove(uint8_t id) {
        uint8_t next = list[id].next;
        uint8_t prev = list[id].prev;

        list[next].prev = prev;
        list[prev].next = next;

        list[id].next = id;
    }

    struct {
        value_type v;
        uint8_t next;
        uint8_t prev;
    } list[N];
};

}
