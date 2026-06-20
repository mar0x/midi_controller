#pragma once

#include <stdint.h>

namespace artl {

template<typename IN_A, typename IN_B, int8_t FACTOR=4>
struct stimer_encoder {
/*
        a  b | ab
        ---------
        0, 0 |  0
        1, 0 |  2
        1, 1 |  3
        0, 1 |  1
        0, 0 |  0
*/

    static constexpr int8_t p2d[4][4] = {
        {  0, -1,  1,  2 },
        {  1,  0,  2, -1 },
        { -1,  2,  0,  1 },
        {  2,  1, -1,  0 },
    };

    static void setup() {
        IN_A::setup();
        IN_B::setup();
        bool a = IN_A::read();
        bool b = IN_B::read();
        ab_ = (a << 1) | b;
        d_ = 0;
        pos_ = 0;
    }

    static int8_t update() {
        bool a = IN_A::read();
        bool b = IN_B::read();

        uint8_t ab = (a << 1) | b;
        int8_t d = p2d[ab_][ab];
        int8_t ret = 0;

        ab_ = ab;

        // guess the direction
        if (d == 2 && d_ < 0) {
            d = -2;
        }

        // heuristic: previous guess was wrong
        if (d_ == -2 && d == 1 && pos_ == -2) {
            pos_ = 2;
        }

        if (d_ == 2 && d == -1 && pos_ == 2) {
            pos_ = -2;
        }

        d_ = d;
        pos_ += d;

        // TODO reset pos_ whan ab == 0

        while (pos_ >= FACTOR) {
            pos_ -= FACTOR;

            ret += 1;
        }

        while (pos_ <= -FACTOR) {
            pos_ += FACTOR;

            ret -= 1;
        }

        // if (ab == 0) { pos_ = 0; }

        return ret;
    }

    static int8_t pos_;
    static int8_t d_;
    static uint8_t ab_;
};

template<typename IN_A, typename IN_B, int8_t FACTOR>
constexpr int8_t stimer_encoder<IN_A, IN_B, FACTOR>::p2d[4][4];

template<typename IN_A, typename IN_B, int8_t FACTOR>
int8_t stimer_encoder<IN_A, IN_B, FACTOR>::pos_;

template<typename IN_A, typename IN_B, int8_t FACTOR>
int8_t stimer_encoder<IN_A, IN_B, FACTOR>::d_;

template<typename IN_A, typename IN_B, int8_t FACTOR>
uint8_t stimer_encoder<IN_A, IN_B, FACTOR>::ab_;

}
