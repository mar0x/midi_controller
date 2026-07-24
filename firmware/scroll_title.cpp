#include "scroll_title.h"

#include "display_1602.h"
#include "screen_buf.h"
#include "stimer.h"

namespace midi_controller {

namespace {

uint8_t cur_title_start;
char cur_title[32];
uint8_t cur_title_size;
uint8_t cur_title_pos;
int8_t cur_title_scroll_dir;

void scroll_title();

}

using screen = screen_buf_t<display_1602>;

void set_title(uint8_t start, const char *title, uint8_t size) {
    cur_title_start = start;
    memcpy(cur_title, title, size);
    uint8_t max_ts = 16 - cur_title_start;

    cur_title_pos = 0;
    cur_title_size = size;
    uint8_t ts = cur_title_size;
    if (ts > max_ts) {
        ts = max_ts;
        cur_title_scroll_dir = 1;
        stimer::schedule_in(STIMER_SCROLL_TITLE,
            SCROLL_TITLE_START_DELAY_MS,
            SCROLL_TITLE_MOVE_DELAY_MS,
            scroll_title);
    } else {
        stimer::cancel(STIMER_SCROLL_TITLE);
    }

    screen::set_cursor(cur_title_start, 0);
    screen::write(cur_title, ts);
    screen::fill();
}

void scroll_title_stop() {
    stimer::cancel(STIMER_SCROLL_TITLE);
}

namespace {

void scroll_title() {
    cur_title_pos += cur_title_scroll_dir;

    bool ch_dir = false;
    uint8_t max_ts = 16 - cur_title_start;
    uint8_t ts = cur_title_size - cur_title_pos;
    if (ts > max_ts) {
        ts = max_ts;
    } else {
        ch_dir = true;
    }

    if (cur_title_pos == 0) {
        ch_dir = true;
    }

    screen::set_cursor(cur_title_start, 0);
    screen::write(cur_title + cur_title_pos, ts);

    if (ch_dir) {
        cur_title_scroll_dir = -cur_title_scroll_dir;
        stimer::schedule_in(STIMER_SCROLL_TITLE,
            SCROLL_TITLE_START_DELAY_MS,
            SCROLL_TITLE_MOVE_DELAY_MS);
    }
}

}

}
