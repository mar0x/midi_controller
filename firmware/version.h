
#pragma once

#define FW_VERSION "2021.12.28"

#include "rotate.h"
#include "lcd.h"

namespace patch_mate {

struct version_t {
    enum {
        MAX_LINE = 5,
    };

    uint8_t line_ = 0;
    const char *lines[MAX_LINE];

    static char sn_[];
    static char hw_[];

    version_t() {
        lines[0] = "FW " FW_VERSION;
        lines[1] = hw_;
        lines[2] = sn_;
        lines[3] = "BD " __DATE__;
        lines[4] = "BT " __TIME__;
    }

    void sn(const char *sn) {
        memcpy(sn_ + 3, sn, 4);
    }

    const char *sn() const { return sn_ + 3; }

    void hw(const char *hw) {
        memcpy(hw_ + 3, hw, 10);
    }

    const char *hw() const { return hw_ + 3; }

    void reset() { line_ = 0; }

    void rotate(int dir) {
        line_ = ::patch_mate::rotate(line_, 0, 5, dir);
    }

    void print_line() const {
        uint8_t n = 0;
        const char *l = lines[line_];

        while (l[n]) { lcd_buf[1][n] = l[n]; ++n; }

        while (n < LCD_COLUMNS) { lcd_buf[1][n++] = ' '; }
    }
};

char version_t::sn_[] = "SN XXXX";
char version_t::hw_[] = "HW XXXX.XX.XX";

}
