
#pragma once

namespace patch_mate {

struct serial_out {
    static void setup() {
        Serial.begin(115200);
    }

    static void serial_write(const uint8_t *b, uint8_t l) {
        if (Serial.dtr()) {
            Serial.write(b, l);
        }
    }

    template<typename T1>
    static void serial_print(const T1& a1) {
        if (Serial.dtr()) {
            Serial.print(a1);
        }
    }

    template<typename T1, typename ...Args>
    static void serial_print(const T1& a1, Args... args) {
        if (Serial.dtr()) {
            Serial.print(a1);
            serial_print(args...);
        }
    }

    template<typename T1>
    static void serial_println(const T1& a1) {
        if (Serial.dtr()) {
            Serial.println(a1);
        }
    }

    template<typename T1, typename ...Args>
    static void serial_println(const T1& a1, Args... args) {
        if (Serial.dtr()) {
            Serial.print(a1);
            serial_println(args...);
        }
    }
};

}
