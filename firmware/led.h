#pragma once

namespace midi_controller {

struct led {
    static void setup();

    static void blink_up();
    static void blink_down();

    static void start_blink_ok();
    static void stop_blink_ok();

    static void ok(bool v);
    static void toggle_ok();
};

}
