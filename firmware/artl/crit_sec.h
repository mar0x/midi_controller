#pragma once

#if defined(ARDUINO_ARCH_AVR)

#include <avr/io.h>
#include <avr/interrupt.h>

#endif

namespace artl
{

struct crit_sec {
    crit_sec() : sreg_(SREG) { cli(); }
    ~crit_sec() { SREG = sreg_; }
private:
    uint8_t sreg_;
};

}
