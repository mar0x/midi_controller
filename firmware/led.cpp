#include "led.h"
#include "stimer.h"
#include "pinout.h"

namespace midi_controller {

namespace {

bool ok_led_state;
bool up_led_state;
bool down_led_state;

void update_ok_led() {
    ok_led_state = !ok_led_state;
    ok_led_pin::write(ok_led_state);
}

void update_up_led() {
    up_led_state = !up_led_state;
    up_led_pin::write(up_led_state);
    if (up_led_state) {
        stimer::schedule_in(STIMER_UP_BLINK, BLINK_DELAY_MS);
    }
}

void update_down_led() {
    down_led_state = !down_led_state;
    down_led_pin::write(down_led_state);
    if (down_led_state) {
        stimer::schedule_in(STIMER_DOWN_BLINK, BLINK_DELAY_MS);
    }
}

}

void led::setup() {
    ok_led_pin::setup();
    up_led_pin::setup();
    left_led_pin::setup();
    down_led_pin::setup();

    stimer::callback(STIMER_OK_BLINK, update_ok_led);
    stimer::callback(STIMER_UP_BLINK, update_up_led);
    stimer::callback(STIMER_DOWN_BLINK, update_down_led);
}

void led::blink_up() {
    if (!stimer::active(STIMER_UP_BLINK)) {
        up_led_state = false;
        stimer::schedule_in(STIMER_UP_BLINK, BLINK_DELAY_MS);
    }
}

void led::blink_down() {
    if (!stimer::active(STIMER_DOWN_BLINK)) {
        down_led_state = false;
        stimer::schedule_in(STIMER_DOWN_BLINK, BLINK_DELAY_MS);
    }
}

void led::start_blink_ok() {
    ok_led_state = false;
    stimer::schedule_in(STIMER_OK_BLINK, FR_OK_BLINK_DELAY_MS, FR_OK_BLINK_DELAY_MS);
}

void led::stop_blink_ok() {
    ok_led_state = false;
    ok_led_pin::low();
    stimer::cancel(STIMER_OK_BLINK);
}

void led::ok(bool v) {
    ok_led_state = v;
    ok_led_pin::write(v);
}

void led::toggle_ok() {
    ok_led_state = !ok_led_state;
    ok_led_pin::write(ok_led_state);
}

}
