#pragma once

#include <stdint.h>
#include <string.h>

/*

PC [<P>]
CC <C> [<V>]

LP [<L> [<V>]]
NM [<NAME>]
ST
RS
MT [1/0]

      3     4      15
PR [ <P> [ <L> [ <NAME> [ <PATCHES> ] ] ] ]
<P> <L> <NAME> [ <PATCHES> ]

MC [ <C> ]
<C>

MPI <L> [ <C> ]
MPO <L> [ <C> ]
MO [ 1/0 ]
MF [ 1/0 ]
DL [ <L> ]
MD [ <ms> ]
HC [ <s> ]
V
E [ 1/0 ]

FR <V>

*/

namespace midi_controller {

struct serial_cmd_t {

    enum {
        CMD_UNKNOWN,
        CMD_PROG_CHANGE,        // PC p
        CMD_CTRL_CHANGE,        // CC c v
        CMD_LOOP,               // LP l v
        CMD_NAME,               // NM n
        CMD_MODE,               // MD m
        CMD_STORE,              // ST
        CMD_RESTORE,            // RS
        CMD_MUTE,               // MT m
        CMD_PROGRAM,            // PR p t n
        CMD_MIDI_CHANNEL,       // MC c
        CMD_MIDI_LOOP_IN_CTRL,  // MLI l c
        CMD_MIDI_LOOP_OUT_CTRL, // MLO l c
        CMD_MIDI_PROG_OUT,      // MO o
        CMD_MIDI_FORWARD,       // MF f
        CMD_DEBUG_LEVEL,        // DL l
        CMD_MUTE_DELAY,         // ML s
        CMD_HIDE_CURSOR_DELAY,  // HC s
        CMD_VERSION,            // V
        CMD_ECHO,               // E e
        CMD_FACTORY_RESET,      // FR v
        CMD_MIDI_MON_IN,        // MMI
        CMD_MIDI_MON_OUT,       // MMO
        CMD_MIDI_DUMP_SEND,     // MDS
        CMD_MIDI_DUMP_RECV,     // MDR
        CMD_HELP,               // ?

        CMD_BTN_PRESS,          // B b
        CMD_DISPLAY_DUMP,       // D

        CMD_SERIAL_NUMBER,      // SN
        CMD_HARDWARE,           // HW

        MAX_SIZE = 50,
        MAX_ARGS = 5,
    };

    serial_cmd_t() : ready_(false), size_(0), command_(CMD_UNKNOWN) { }

    void read(uint8_t b);

    operator bool() const { return ready_; }

    void reset() {
        ready_ = false;
        size_ = 0;
        command_ = CMD_UNKNOWN;
    }

    uint8_t command() const { return command_; }

    template<typename T>
    bool get_hex(uint8_t s, uint8_t e, T& v);

    template<typename T>
    bool get_dec(uint8_t s, uint8_t e, T& v);

    template<typename T>
    bool get_bin(uint8_t s, uint8_t e, T& v);

    template<typename T>
    bool get_num(uint8_t n, T& v);

    bool get_arg(uint8_t n, uint8_t& v) {
        return get_num(n, v);
    }

    bool get_arg(uint8_t n, uint16_t& v) {
        return get_num(n, v);
    }

    template<typename T>
    bool get_arg(uint8_t n, T& v);

//private:
    void parse();

    struct arg {
        uint8_t start;
        uint8_t end;
    };

    char buf_[MAX_SIZE];
    bool ready_;
    uint8_t size_;
    uint8_t command_;

    arg arg_[MAX_ARGS];
    uint8_t arg_size_;
};

}

namespace midi_controller {

template<typename T>
bool serial_cmd_t::get_hex(uint8_t s, uint8_t e, T& v) {
    for (uint8_t p = s; p < e; ++p) {
        if (buf_[p] >= '0' && buf_[p] <= '9') {
            v = v * 16 + (buf_[p] - '0');
        } if (buf_[p] >= 'a' && buf_[p] <= 'f') {
            v = v * 16 + (10 + buf_[p] - 'a');
        } if (buf_[p] >= 'A' && buf_[p] <= 'F') {
            v = v * 16 + (10 + buf_[p] - 'A');
        } else {
            return false;
        }
    }

    return true;
}

template<typename T>
bool serial_cmd_t::get_dec(uint8_t s, uint8_t e, T& v) {
    for (uint8_t p = s; p < e; ++p) {
        if (buf_[p] >= '0' && buf_[p] <= '9') {
            v = v * 10 + (buf_[p] - '0');
        } else {
            return false;
        }
    }

    return true;
}

template<typename T>
bool serial_cmd_t::get_bin(uint8_t s, uint8_t e, T& v) {
    for (uint8_t p = s; p < e; ++p) {
        if (buf_[p] >= '0' && buf_[p] <= '1') {
            v = (v << 1) + (buf_[p] - '0');
        } else {
            return false;
        }
    }

    return true;
}

template<typename T>
bool serial_cmd_t::get_num(uint8_t n, T& v) {
    if (n >= arg_size_) {
        return false;
    }

    arg *a = &arg_[n];

    v = 0;

    if (a->end - a->start > 2) {
        if (buf_[a->start] == '0' && (buf_[a->start + 1] == 'x' || buf_[a->start + 1] == 'X')) {
            return get_hex(a->start + 2, a->end, v);
        }
    }

    if (a->end - a->start > 1) {
        if (buf_[a->end - 1] == 'h' || buf_[a->end - 1] == 'H') {
            return get_hex(a->start, a->end - 1, v);
        }

        if (buf_[a->end - 1] == 'b' || buf_[a->end - 1] == 'B') {
            return get_bin(a->start, a->end - 1, v);
        }
    }

    n = 1;

    for (uint8_t p = a->start; p < a->end; ++p) {
        if (buf_[p] >= '0' && buf_[p] <= '1') {
            v = v + ((buf_[p] - '0') ? n : 0);
            n = n << 1;
        } else {
            v = 0;
            break;
        }

        ++p;
        if (p >= a->end) return true;

        if (buf_[p] != ',') {
            v = 0;
            break;
        }
    }

    return get_dec(a->start, a->end, v);
}

template<typename T>
bool serial_cmd_t::get_arg(uint8_t n, T& v) {
    if (n >= arg_size_) {
        return false;
    }

    uint8_t l = arg_[n].end - arg_[n].start;
    uint8_t c = (l > sizeof(T)) ? sizeof(T) : l;

    memcpy(&v, &buf_[arg_[n].start], c);

    if (l < sizeof(T)) {
        memset(((uint8_t *) &v) + l, ' ', sizeof(T) - l);
    }

    return true;
}

}
