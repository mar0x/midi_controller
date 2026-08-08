#include "version.h"
#include "serial_num.h"
#include "hardware_id.h"
#include "storage.h"
#include "config.h"
#include <avr/pgmspace.h>

namespace midi_controller {

namespace {

serial_num_t sn_buf;
hardware_id_t hw_buf;

const char fw_[] PROGMEM = FW_VERSION;
const char build_date_[] PROGMEM = "BD " __DATE__;
const char build_time_[] PROGMEM = "BT " __TIME__;

}

#define SN_DEFAULT "0000"

const char * version_t::serial_no() {
    storage::read(sn_buf);
    if (sn_buf.empty()) {
        memcpy_P(sn_buf, PSTR(SN_DEFAULT), sizeof(SN_DEFAULT));
    }
    return sn_buf;
}

#if HW0
#define HW_DEFAULT "0"
#else
#define HW_DEFAULT "2026.05.29"
#endif

const char * version_t::hw_version() {
    storage::read(hw_buf);
    if (hw_buf.empty()) {
        memcpy_P(hw_buf, PSTR(HW_DEFAULT), sizeof(HW_DEFAULT));
    }
    return hw_buf;
}

prog_var_t<fw_version_t> version_t::fw_version() {
    return fw_;
}

prog_var_t<build_date_t> version_t::build_date() {
    return build_date_;
}

prog_var_t<build_time_t> version_t::build_time() {
    return build_time_;
}

}
