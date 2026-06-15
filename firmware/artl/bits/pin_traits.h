
#pragma once

#include <stdint.h>

#include "port_traits.h"

namespace artl {

namespace pin {

  template<typename PORT, uint8_t BIT_NO, typename PORT_TRAITS = port::traits<PORT>>
  struct traits {

      using port_t = PORT;
      using port_traits = PORT_TRAITS;

      constexpr static uint8_t bit_mask = 1 << BIT_NO;
      constexpr static uint8_t bit_no = BIT_NO;

      static void input() { port_traits::dirclr(bit_mask); }

      static void output() { port_traits::dirset(bit_mask); }

      static bool read() { return read_bit() != 0; }

      static uint8_t read_bit() {
          return port_traits::in() & bit_mask;
      }

      static void write(bool v) {
          if (v) {
              high();
          } else {
              low();
          }
      }

      static void high() { port_traits::outset(bit_mask); }

      static void low() { port_traits::outclr(bit_mask); }

      static void toggle() { port_traits::outtgl(bit_mask); }
  };

  template<uint8_t BIT_NO>
  struct traits<port::DUMMY_LOW, BIT_NO, port::traits<port::DUMMY_LOW> > {

      using port_t = port::DUMMY_LOW;
      using port_traits = port::traits<port_t>;

      constexpr static uint8_t bit_mask = 1 << BIT_NO;
      constexpr static uint8_t bit_no = BIT_NO;

      static void input() { }

      static void output() { }

      static bool read() { return false; }

      static uint8_t read_bit() { return 0; }

      static void write(bool v) { (void) v; }

      static void high() { }

      static void low() { }

      static void toggle() { }
  };

  template<uint8_t BIT_NO>
  struct traits<port::DUMMY_HIGH, BIT_NO, port::traits<port::DUMMY_HIGH> > {

      using port_t = port::DUMMY_HIGH;
      using port_traits = port::traits<port_t>;

      constexpr static uint8_t bit_mask = 1 << BIT_NO;
      constexpr static uint8_t bit_no = BIT_NO;

      static void input() { }

      static void output() { }

      static bool read() { return true; }

      static uint8_t read_bit() { return bit_mask; }

      static void write(bool v) { (void) v; }

      static void high() { }

      static void low() { }

      static void toggle() { }
  };

} // namespace pin

} // namespace artl
