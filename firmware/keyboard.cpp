#include "pinout.h"
#include "stimer.h"
#include "keyboard.h"

#include "artl/delayed_in.h"

using namespace midi_controller;

namespace {

struct ok_btn_cb {
    static void rise();
    static void fall();
};

using ok_delayed = artl::delayed_in<
                          ok_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_OK, BTN_DELAY_MS>,
                          ok_btn_cb>;

struct up_btn_cb {
    static void rise();
    static void fall();
};

using up_delayed = artl::delayed_in<
                          up_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_UP, BTN_DELAY_MS>,
                          up_btn_cb>;

struct down_btn_cb {
    static void rise();
    static void fall();
};

using down_delayed = artl::delayed_in<
                          down_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_DOWN, BTN_DELAY_MS>,
                          down_btn_cb>;

struct left_btn_cb {
    static void rise();
    static void fall();
};

using left_delayed = artl::delayed_in<
                          left_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_LEFT, BTN_DELAY_MS>,
                          left_btn_cb>;

struct right_btn_cb {
    static void rise();
    static void fall();
};

using right_delayed = artl::delayed_in<
                          right_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_RIGHT, BTN_DELAY_MS>,
                          right_btn_cb>;

}

namespace midi_controller {

void keyboard::setup() {
    ok_delayed::setup();

    up_delayed::setup();
    down_delayed::setup();
    left_delayed::setup();
    right_delayed::setup();

    ok_btn_pin::pullup();

    up_btn_pin::pullup();
    down_btn_pin::pullup();
    left_btn_pin::pullup();
    right_btn_pin::pullup();
}

void keyboard::update() {
    ok_delayed::update();

    up_delayed::update();
    down_delayed::update();
    left_delayed::update();
    right_delayed::update();
}

}

namespace {

void ok_btn_cb::rise() { stimer::cancel(STIMER_HOLD_OK); }
void ok_btn_cb::fall() {
    keyboard::on_ok_press();
    stimer::schedule_in(STIMER_HOLD_OK, BTN_HOLD_MS, keyboard::on_ok_hold);
}

void up_btn_cb::rise() { }
void up_btn_cb::fall() { keyboard::on_up_press(); }

void down_btn_cb::rise() { }
void down_btn_cb::fall() { keyboard::on_down_press(); }

void left_btn_cb::rise() { }
void left_btn_cb::fall() { }

void right_btn_cb::rise() { }
void right_btn_cb::fall() { }

}
