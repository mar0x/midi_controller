#pragma once

#include "artl/digital_out.h"
#include "artl/digital_in.h"
#include "artl/digital_pin.h"
#include "artl/active_traits.h"

namespace midi_controller {

using spi_mosi = artl::digital_out<artl::port::B, 1>;
using spi_sck = artl::digital_out<artl::port::B, 2>;
using spi_miso = artl::digital_in<artl::port::B, 3>;
using spi_ss = artl::digital_out<artl::port::B, 0>;

using right_btn_pin = artl::digital_in<artl::port::F, 4>; // PF4 (21/A3)
using ok_btn_pin = artl::digital_in<artl::port::B, 6>; // PB6 (10/A10)
using up_btn_pin = artl::digital_in<artl::port::D, 0>; // PD0 (3)
using left_btn_pin = artl::digital_in<artl::port::C, 6>; // PC6 (5)
using down_btn_pin = artl::digital_in<artl::port::D, 7>; // PD7 (6/A7)
using enc_a_pin = artl::digital_in<artl::port::B, 4>; // PB4 (8/A8)
using enc_b_pin = artl::digital_in<artl::port::B, 5>; // PB5 (9/A9)

using ok_led_pin = artl::digital_out<artl::port::F, 5>; // PF5 (20/A2)
using up_led_pin = artl::digital_out<artl::port::D, 1>; // PD1 (2)
using left_led_pin = artl::digital_out<artl::port::D, 4>; // PD4 (4/A6)
using down_led_pin = artl::digital_out<artl::port::E, 6>; // PE6 (7)

using eeprom_cs_pin = artl::digital_pin<artl::port::F, 6>; // PF6 (19/A1)
using oled_cs_pin = artl::digital_pin<artl::port::F, 7>; // PF7 (18/A0)

using eeprom_cs = artl::active_low<eeprom_cs_pin>;
using oled_cs = artl::active_low<oled_cs_pin>;

}
