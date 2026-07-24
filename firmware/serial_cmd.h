#pragma once

#include <stdint.h>
#include <string.h>

namespace midi_controller {

enum {
    SCMD_UNKNOWN,
    SCMD_PROGRAM_CHANGE,     // PC f p
    SCMD_PROGRAM,            // PR f p "n" s
    SCMD_PROFILE,            // PF f "n" c p

    SCMD_DISPLAY_DUMP,       // D s

    SCMD_MIDI_CHANNEL,       // MC c
    SCMD_PROGRAM_START,      // PS 0/1
    SCMD_CHANNEL_START,      // CS 0/1

    SCMD_DEVICE_TITLE,       // DT "t"
    SCMD_FACTORY_DATA,       // FD l

    SCMD_SERIAL_NUMBER,      // SN
    SCMD_HARDWARE,           // HW
    SCMD_VERSION,            // V
    SCMD_HELP,               // ?

    SCMD_NAME,               // NM n
    SCMD_MODE,               // MD m
    SCMD_STORE,              // ST
    SCMD_RESTORE,            // RS
    SCMD_MIDI_PROG_OUT,      // MO o
    SCMD_MIDI_FORWARD,       // MF f
    SCMD_DEBUG_LEVEL,        // DL l
    SCMD_HIDE_CURSOR_DELAY,  // HC s
    SCMD_ECHO,               // E e
    SCMD_FACTORY_RESET,      // FR v
    SCMD_MIDI_MON_IN,        // MMI
    SCMD_MIDI_MON_OUT,       // MMO
    SCMD_MIDI_DUMP_SEND,     // MDS
    SCMD_MIDI_DUMP_RECV,     // MDR

    SCMD_BTN_PRESS,          // B b

};

struct serial_cmd_t {

    enum {
        MAX_SIZE = 50,
        MAX_ARGS = 5,
    };

    serial_cmd_t() : ready_(false), size_(0), command_(SCMD_UNKNOWN) { }

    void read(uint8_t b);

    operator bool() const { return ready_; }

    void reset() {
        ready_ = false;
        size_ = 0;
        command_ = SCMD_UNKNOWN;
    }

    uint8_t command() const { return command_; }

    template<typename T>
    bool get_hex(uint8_t s, uint8_t e, T& v) const;

    template<typename T>
    bool get_dec(uint8_t s, uint8_t e, T& v) const;

    template<typename T>
    bool get_bin(uint8_t s, uint8_t e, T& v) const;

    template<typename T>
    bool get_num(uint8_t n, T& v) const;

    bool get_arg(uint8_t n, uint8_t& v) const {
        return get_num(n, v);
    }

    bool get_arg(uint8_t n, uint16_t& v) const {
        return get_num(n, v);
    }

    template<typename T>
    bool get_arg(uint8_t n, T& v) const;

//private:
    void parse();

    struct arg {
        uint8_t start;
        uint8_t end;

        uint8_t size() const { return end - start; }
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
bool serial_cmd_t::get_hex(uint8_t s, uint8_t e, T& v) const {
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
bool serial_cmd_t::get_dec(uint8_t s, uint8_t e, T& v) const {
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
bool serial_cmd_t::get_bin(uint8_t s, uint8_t e, T& v) const {
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
bool serial_cmd_t::get_num(uint8_t n, T& v) const {
    if (n >= arg_size_) {
        return false;
    }

    const arg &a = arg_[n];
    uint8_t s = a.start;
    uint8_t e = a.end;

    v = 0;

    if (e - s > 2) {
        if (buf_[s] == '0' && (buf_[s + 1] == 'x' || buf_[s + 1] == 'X')) {
            return get_hex(s + 2, e, v);
        }
    }

    if (e - s > 1) {
        if (buf_[e - 1] == 'h' || buf_[e - 1] == 'H') {
            return get_hex(s, e - 1, v);
        }

        if (buf_[e - 1] == 'b' || buf_[e - 1] == 'B') {
            return get_bin(s, e - 1, v);
        }
    }

    n = 1;

    for (uint8_t p = s; p < e; ++p) {
        if (buf_[p] >= '0' && buf_[p] <= '1') {
            v = v + ((buf_[p] - '0') ? n : 0);
            n = n << 1;
        } else {
            v = 0;
            break;
        }

        ++p;
        if (p >= e) return true;

        if (buf_[p] != ',') {
            v = 0;
            break;
        }
    }

    return get_dec(s, e, v);
}

template<typename T>
bool serial_cmd_t::get_arg(uint8_t n, T& v) const {
    if (n >= arg_size_) {
        return false;
    }

    uint8_t l = arg_[n].end - arg_[n].start;
    uint8_t c = (l > sizeof(T) - 1) ? sizeof(T) - 1 : l;

    memcpy(&v, &buf_[arg_[n].start], c);
    memset(((uint8_t *) &v) + l, 0, sizeof(T) - l);

    return true;
}

}
