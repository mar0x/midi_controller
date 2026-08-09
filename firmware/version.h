#pragma once

#define FW_VERSION "2026.08.09"

#include "prog_var.h"

namespace midi_controller {

using fw_version_t = char[sizeof(FW_VERSION)];
using build_date_t = char[sizeof("BD " __DATE__)];
using build_time_t = char[sizeof("BT " __TIME__)];

struct version_t {
    static const char * serial_no();
    static const char * hw_version();

    static prog_var_t<fw_version_t> fw_version();
    static prog_var_t<build_date_t> build_date();
    static prog_var_t<build_time_t> build_time();
};

}
