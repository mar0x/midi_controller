#include "encoder.h"
#include "pinout.h"
#include "stimer.h"

#include "artl/delayed_in.h"
#include "artl/stimer_encoder.h"

using namespace midi_controller;

namespace {

struct enc_cb {
    static uint16_t pos;

    static void rise() { update(); }
    static void fall() { update(); }

    static void update();
};

using enc_a_delayed = artl::delayed_in<
                          enc_a_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_ENC_A, 3>,
                          enc_cb>;

using enc_b_delayed = artl::delayed_in<
                          enc_b_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_ENC_B, 3>,
                          enc_cb>;

using enc = artl::stimer_encoder<enc_a_delayed, enc_b_delayed, 4>;

}

namespace midi_controller {

void encoder::setup() {
    enc::setup();
}

void encoder::update() {
    enc_a_delayed::update();
    enc_b_delayed::update();
}

}

namespace {

uint16_t enc_cb::pos = 0;

void enc_cb::update() {
    int8_t ret = enc::update();

    if (ret) {
        encoder::on_rotate(ret);
    }

    pos += ret;
}

}


#if defined(ISR) && defined(PCINT0_vect)
ISR(PCINT0_vect) {
}
#endif
