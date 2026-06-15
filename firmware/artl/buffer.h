#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#if !defined(ARDUINO_ARCH_AVR)
char *ltoa(long n, char *out, int d);
char *ultoa(unsigned long n, char *out, int d);
char *dtostrf(float f, int w, int d, char *out);
#endif

namespace artl {

struct float_fmt_t {
    float_fmt_t(float val, uint8_t min_width = 4, uint8_t dec_digits = 2) :
        v(val), w(min_width), d(dec_digits) { }

    float v;
    uint8_t w;
    uint8_t d;
};

struct buffer_t {
    using size_type = uint8_t;

    buffer_t& append(const char* s, size_type size) {
        if (size >= capacity - len) {
            size = capacity - len - 1;
        }
        memcpy(buf + len, s, size);
        buf[len + size] = 0;
        len += size;

        return *this;
    }

    buffer_t& append(const char* s) {
        size_type size = strlen(s);
        if (size >= capacity - len) {
            size = capacity - len - 1;
        }
        memcpy(buf + len, s, size + 1);
        len += size;
        return *this;
    }

    buffer_t& append(uint32_t n) {
        char str[32];
        return append(ultoa(n, str, 10));
    }

    buffer_t& append(int32_t n) {
        char str[32];
        return append(ltoa(n, str, 10));
    }

    buffer_t& append(char c) {
        if (len < capacity - 1) {
            buf[len] = c;
            ++len;
            buf[len] = 0;
        }

        return *this;
    }

    buffer_t& append(int16_t n) {
        return append((int32_t) n);
    }

    buffer_t& append(uint16_t n) {
        return append((uint32_t) n);
    }

    buffer_t& append(float d) {
        return append(float_fmt_t(d));
    }

    buffer_t& append(const float_fmt_t& ff) {
        char str[32];
        return append(dtostrf(ff.v, ff.w, ff.d, str));
    }

    buffer_t& assign(const char* s) {
        return reset().append(s);
    }

    buffer_t& operator<<(char c) { return append(c); }
    buffer_t& operator<<(const char* s) { return append(s); }
    buffer_t& operator<<(int32_t n) { return append(n); }
    buffer_t& operator<<(uint32_t n) { return append(n); }
    buffer_t& operator<<(int16_t n) { return append(n); }
    buffer_t& operator<<(uint16_t n) { return append(n); }
    buffer_t& operator<<(float n) { return append(n); }
    buffer_t& operator<<(const float_fmt_t& n) { return append(n); }

    buffer_t& operator+=(char c) { return append(c); }
    buffer_t& operator+=(const char* s) { return append(s); }
    buffer_t& operator+=(int32_t n) { return append(n); }
    buffer_t& operator+=(uint32_t n) { return append(n); }
    buffer_t& operator+=(int16_t n) { return append(n); }
    buffer_t& operator+=(uint16_t n) { return append(n); }
    buffer_t& operator+=(float n) { return append(n); }
    buffer_t& operator+=(const float_fmt_t& n) { return append(n); }

    buffer_t& operator=(const char* s) { return assign(s); }

    uint8_t operator[](uint8_t i) const { return buf[i]; }
/*
    void remove(size_type pos, size_type size) {
        size_type l = (pos + size >= len) ? 0 : len - (pos + size);
        memmove(buf + pos, buf + pos + size, l);
        len = pos + l;
    }
*/

    void attach(size_type n, uint8_t *b) {
        len = 0;
        buf = b;
        capacity = n;
    }

    buffer_t& reset() {
        len = 0;
        buf[0] = 0;
        return *this;
    }

    const char* c_str() const { return (char *) buf; }
    const uint8_t* data() const { return buf; }
    size_type length() const { return len; }
    bool empty() const { return len == 0; }
    bool full() const { return len + 1 >= capacity; }

    uint8_t *buf = NULL;
    size_type len = 0;
    size_type capacity = 0;
};

}
