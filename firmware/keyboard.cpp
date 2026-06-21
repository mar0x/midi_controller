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
                          artl::stimer_scheduler<stimer, STIMER_DELAY_OK, 5>,
                          ok_btn_cb>;

struct up_btn_cb {
    static void rise();
    static void fall();
};

using up_delayed = artl::delayed_in<
                          up_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_UP, 5>,
                          up_btn_cb>;

struct down_btn_cb {
    static void rise();
    static void fall();
};

using down_delayed = artl::delayed_in<
                          down_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_DOWN, 5>,
                          down_btn_cb>;

struct left_btn_cb {
    static void rise();
    static void fall();
};

using left_delayed = artl::delayed_in<
                          left_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_LEFT, 5>,
                          left_btn_cb>;

struct right_btn_cb {
    static void rise();
    static void fall();
};

using right_delayed = artl::delayed_in<
                          right_btn_pin,
                          artl::stimer_scheduler<stimer, STIMER_DELAY_RIGHT, 5>,
                          right_btn_cb>;

}

namespace midi_controller {

void keyboard::setup() {
    ok_delayed::setup();

    up_delayed::setup();
    down_delayed::setup();
    left_delayed::setup();
    right_delayed::setup();
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

void ok_btn_cb::rise() { }
void ok_btn_cb::fall() { }

void up_btn_cb::rise() { }
void up_btn_cb::fall() { keyboard::on_up_press(); }

void down_btn_cb::rise() { }
void down_btn_cb::fall() { }

void left_btn_cb::rise() { }
void left_btn_cb::fall() { }

void right_btn_cb::rise() { }
void right_btn_cb::fall() { }

}
