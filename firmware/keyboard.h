#pragma once

namespace midi_controller {

struct keyboard {
    static void setup();
    static void update();

    static void on_up_press();
    static void on_down_press();
    static void on_ok_press();
    static void on_ok_hold();
};

}
