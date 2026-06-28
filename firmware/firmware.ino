#include "pinout.h"
#include "spi.h"
#include "display_1602.h"
#include "screen_buf.h"

#include "encoder.h"
#include "keyboard.h"
#include "stimer.h"
#include "artl/yield.h"

#include "settings.h"
#include "program.h"
#include "midi_cmd.h"
#include "spi_eeprom.h"
#include "serial_cmd.h"

using namespace midi_controller;

const uint16_t CLOCK_PERIOD = 1000;

enum {
    MODE_INIT,
    MODE_NORMAL,
    MODE_SETTINGS_FIRST,
    MODE_SETTINGS_CHANNEL = MODE_SETTINGS_FIRST,
    MODE_SETTINGS_PROG_OUT,
    MODE_SETTINGS_MIDI_FWD,
    MODE_SETTINGS_HIDE_CURSOR,
    MODE_SETTINGS_PROG_START,
#if defined(DEBUG)
    MODE_SETTINGS_USB_DEBUG,
#endif
    MODE_SETTINGS_FACTORY_RESET,
    MODE_SETTINGS_LAST = MODE_SETTINGS_FACTORY_RESET,
    MODE_PROGRAM_SWAP,
    MODE_PROGRAM_MOVE,
    MODE_MIDI_IN_MONITOR,
    MODE_MIDI_OUT_MONITOR,
    MODE_MIDI_DUMP_SEND,
    MODE_MIDI_DUMP_RECV,
    MODE_ABOUT,
    MODE_UPTIME,
    MODE_MAX,
};

using screen = screen_buf_t<display_1602>;

struct display_text_t {
    uint8_t row:4;
    uint8_t col:4;
    const char *str;
};

uint8_t write_text(const display_text_t &t) {
    if (t.str) {
        screen::set_cursor(t.col, t.row);
        return screen::write(t.str);
    }

    return 0;
}

struct mode_desc_t;

struct mode_desc_t {
    // uint8_t mode;
    const display_text_t *text;
    void (*enter)(mode_desc_t *mode);
    void (*leave)(mode_desc_t *mode);
    void (*onkey)(mode_desc_t *mode, uint8_t key);
};

void default_mode_enter(struct mode_desc_t *mode) {
    const display_text_t *t = mode->text;

    while (t && t->str) {
        screen::set_cursor(t->col, t->row);
        screen::write(t->str);
        ++t;
    }
}

void default_mode_leave(mode_desc_t * /* mode */) {
}

void default_mode_onkey(mode_desc_t * /* mode */, uint8_t /* key */) {
}

struct pgm_ptr {
    char read() const {
        return pgm_read_byte(ptr);
    }

    pgm_ptr& operator+=(int8_t i) {
        ptr += i;
        return *this;
    }

    const char *ptr;
};

const char *settings_text = "Settings";
const char *banner_text = "MIDI Controller ";
const display_text_t DT_END = { 0, 0, nullptr };
const display_text_t init_text[] = { { 0, 0, "Mode INIT" }, DT_END };
const display_text_t normal_text[] = { { 0, 0, "Mode NORMAL" }, DT_END };
const display_text_t settings_channel_text[] = { { 0, 0, settings_text }, { 1, 0, "MIDI Channel " }, DT_END };

mode_desc_t mode[] = {
    /* MODE_INIT */ { init_text,  default_mode_enter, default_mode_leave, default_mode_onkey },
    /* MODE_NORMAL */ { normal_text , default_mode_enter, default_mode_leave, default_mode_onkey },
    /* MODE_SETTINGS_CHANNEL */ { settings_channel_text , default_mode_enter, default_mode_leave, default_mode_onkey },
};

uint8_t mode_;

void set_mode(uint8_t m) {
    if (m == mode_) {
        return;
    }

    if (m > MODE_SETTINGS_CHANNEL) {
        return;
    }

    if (mode[mode_].leave) {
        mode[mode_].leave(mode + mode_);
    }

    if (mode[m].enter) {
        mode[m].enter(mode + m);
    }

    mode_ = m;
}

uint16_t cur_prog_id_ = 0;
program_t cur_prog_;
uint8_t cur_title_start;
uint8_t cur_title_size;
uint8_t cur_title_pos;
int8_t cur_title_scroll_dir;

enum {
    MAX_PROGRAM = 200,
    PROGRAM_START = 32,
};

settings_t settings_;

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
    screen::write(cur_prog_.title() + cur_title_pos, ts);

    if (ch_dir) {
        cur_title_scroll_dir = -cur_title_scroll_dir;
        stimer::schedule_in(STIMER_SCROLL_TITLE, 1000, 500);
    }
}

void set_program(uint16_t p, bool show_banner = false) {
    if (p >= MAX_PROGRAM) return;

    cur_prog_id_ = p;

    spi_eeprom::get(PROGRAM_START + p * sizeof(program_t), cur_prog_);

    uint16_t n = cur_prog_id_ + settings_.prog_start;
    uint8_t n_size = 1;
    if (n > 9) n_size += 1;
    if (n > 99) n_size += 1;
    cur_title_start = n_size + 1;
    uint8_t max_ts = 16 - cur_title_start;

    cur_title_pos = 0;
    cur_title_size = cur_prog_.title_size();
    uint8_t ts = cur_title_size;
    if (ts > max_ts) {
        ts = max_ts;
        cur_title_scroll_dir = 1;
        stimer::schedule_in(STIMER_SCROLL_TITLE, 1000, 500, scroll_title);
    } else {
        stimer::cancel(STIMER_SCROLL_TITLE);
    }

    screen::set_cursor(0, 0);
    screen::print_num(n_size, n);
    screen::write(' ');
    screen::write(cur_prog_.title(), ts);
    for (uint8_t i = ts; i < max_ts; ++i) {
        screen::write(' ');
    }

    if (!show_banner) {
        uint8_t bank_no = cur_prog_id_ / 2;
        uint8_t bank_ab = cur_prog_id_ % 2;

        screen::set_cursor(0, 1);
        screen::write("BANK", 4);
        screen::print_num(2, bank_no, '0');
        screen::write(' ');
        screen::write('A' + bank_ab);
        for (uint8_t i = 8; i < 16; ++i) {
            screen::write(' ');
        }
    } else {
        screen::set_cursor(0, 1);
        screen::write(banner_text);
    }

    uint8_t cc0 = p < 128 ? 0 : 1;
    p = p < 128 ? p : p - 128;

    midi_cmd_t cmd0(settings_.midi_channel, CMD_CTRL_CHANGE, 0, cc0);
    midi_cmd_t cmd1(settings_.midi_channel, CMD_PROG_CHANGE, p);

    Serial1.write(&cmd0, cmd0.size());
    Serial1.write(&cmd1, cmd1.size());
}

namespace midi_controller {

void encoder::on_rotate(int8_t d) {
    uint16_t p = (cur_prog_id_ + MAX_PROGRAM + d) % MAX_PROGRAM;

    set_program(p);
}

void keyboard::on_up_press() {
    uint8_t m = (mode_ + 1) % 3;

    if (Serial.dtr()) {
        Serial.println("up");
    }
    set_mode(m);
}

}

void update_clock() {
/*
    if (Serial.dtr()) {
        Serial.print(millis());
        Serial.print(" left ");
        Serial.print(left_btn_pin::read());

        Serial.print(", right ");
        Serial.print(right_btn_pin::read());

        Serial.print(", up ");
        Serial.print(up_btn_pin::read());

        Serial.print(", down ");
        Serial.print(down_btn_pin::read());

        Serial.print(", ok ");
        Serial.println(ok_btn_pin::read());
    }
*/
}

serial_cmd_t serial_cmd;

void setup() {
    stimer::setup();

    keyboard::setup();
    encoder::setup();

    ok_led_pin::setup();
    up_led_pin::setup();
    left_led_pin::setup();
    down_led_pin::setup();

    spi::init();

    spi_eeprom::setup();
    screen::setup();

    //display_1602::begin();
    //display_1602::clear();
    //display_1602::write_pgm(PSTR("Hello PSTR"));
    //display_1602::cursor(true);

    stimer::schedule_in(STIMER_CLOCK, 0, CLOCK_PERIOD, update_clock);

    Serial1.begin(31250);
    Serial.begin(115200);

    set_program(0, true);
}

void loop() {
    clk_t::clock_source::update();

    stimer::update();

    keyboard::update();
    encoder::update();

    if (Serial.available()) {
        int c = Serial.read();
        serial_cmd.read(c);

        if (settings_.echo) {
            Serial.write(c);
        }

        if (serial_cmd) {
            if (serial_cmd.command() == SCMD_PROGRAM) {
                program_t pr = cur_prog_;
                uint16_t p = cur_prog_id_;

                if (serial_cmd.get_arg(1, p)) {
                    if (p >= MAX_PROGRAM) p = cur_prog_id_;

                    if (p != cur_prog_id_) {
                        spi_eeprom::get(PROGRAM_START + p * sizeof(program_t), pr);
                    }

                    if (serial_cmd.arg_size_ > 2) {
                        serial_cmd_t::arg &t = serial_cmd.arg_[2];
                        pr.reset();
                        pr.title(serial_cmd.buf_ + t.start, t.size());

                        spi_eeprom::put(PROGRAM_START + p * sizeof(program_t), pr);

                        if (p == cur_prog_id_) {
                            set_program(p);
                        }
                    }
                }

                Serial.print("PR ");
                Serial.print(p);
                Serial.print(" \"");
                Serial.print(pr.title());
                Serial.println("\"");
            }

            serial_cmd.reset();
        }
    }

    // ok_led_pin::write(!ok_btn_pin::read());
    up_led_pin::write(!up_btn_pin::read());
    down_led_pin::write(!down_btn_pin::read());
    left_led_pin::write(!left_btn_pin::read());

    artl::yield();
}
