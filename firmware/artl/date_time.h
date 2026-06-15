#pragma once

#include "buffer.h"

namespace artl {

// Binary Coded Decimal
struct bcd_value {
    void str(char *s) const {
        s[0] = '0' + (v >> 4);
        s[1] = '0' + (v & 0x0F);
    }

    buffer_t& append_to(buffer_t& b) const {
        str((char *) b.buf + b.len);
        b.len += 2;
        b.buf[b.len] = 0;
        return b;
    }

    bcd_value& operator=(uint8_t d) { v = d; return *this; }
    void set(const char *s) { v = ((s[0] - '0') << 4) | (s[1] - '0'); }

    explicit operator uint8_t() const { return 10 * (v >> 4) + (v & 0x0F); }

    bool operator==(const bcd_value& a) const { return v == a.v; }
    bool operator!=(const bcd_value& a) const { return v != a.v; }

    uint8_t v;
};

struct date_time_t {
    bcd_value year() const { return y; }
    bcd_value month() const { return m; }
    bcd_value day() const { return d; }

    bcd_value hour() const { return hh; }
    bcd_value minute() const { return mm; }
    bcd_value second() const { return ss; }

    bcd_value operator[](uint8_t n) const { return (&ss)[n]; }
    bcd_value& operator[](uint8_t n) { return (&ss)[n]; }
    void set(uint8_t n, bcd_value v) { (&ss)[n] = v; }
    void set(const char *s) {
        y.set(s);
        m.set(s + 2);
        d.set(s + 4);
        hh.set(s + 7);
        mm.set(s + 9);
        ss.set(s + 11);
    }

    bcd_value ss, mm, hh, dummy, d, m, y;
};

namespace {

inline
buffer_t& operator += (buffer_t& s, bcd_value b) {
    return b.append_to(s);
}

inline
buffer_t& operator << (buffer_t& s, bcd_value b) {
    return b.append_to(s);
}

inline
buffer_t& operator << (buffer_t& s, const date_time_t& d) {
    return s << d.year() << d.month() << d.day() << '-'
        << d.hour() << d.minute() << d.second();
}

}

}
