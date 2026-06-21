#pragma once

namespace midi_controller {

struct keyboard {
    static void setup();
    static void update();

    static void on_up_press();
};

}
