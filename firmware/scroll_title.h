#pragma once

#include <stdint.h>

namespace midi_controller {

void set_title(uint8_t start, const char *title, uint8_t size);
void scroll_title_stop();

}
