#include "pinout.h"
#include "spi.h"
#include "display_1602.h"
#include "screen_buf.h"
#include "config.h"

#include "encoder.h"
#include "keyboard.h"
#include "stimer.h"
#include "artl/yield.h"

#include "settings.h"
#include "profile.h"
#include "program.h"
#include "midi_cmd.h"
#include "storage.h"
#include "serial_cmd.h"

#include "mode.h"
#include "current_state.h"
#include "scroll_title.h"
#include "led.h"
#include "screen.h"

#include "version.h"

using namespace midi_controller;

namespace midi_controller {

const uint16_t CLOCK_PERIOD = 1000;

uint8_t cur_profile_id;
profile_t cur_profile;
uint8_t select_profile_id;

uint8_t cur_program_id;
uint8_t cur_program_seq;
program_t cur_program;

void show_program(uint8_t p, uint8_t s, bool show_banner) {
    if (p >= MAX_PROGRAM) return;

    if (s >= MAX_PROGRAM) {
        s = storage::program_seq(cur_profile_id, p);
    }

    program_t pr;

    if (p == cur_program_id) {
        pr = cur_program;
    } else {
        storage::read(cur_profile_id, p, pr);
    }

    uint8_t n = s + settings.program_start;
    uint8_t n_size = 1;
    if (n > 9) n_size += 1;
    if (n > 99) n_size += 1;

    screen::set_cursor(0, 0);
    screen::print_num(n_size, n);
    screen::write(' ');

    set_title(n_size + 1, pr.title(), pr.title_size());

    if (!show_banner) {
        switch (cur_profile_id) {
        case PROFILE_MOBIUS: {
            uint8_t bank_no = cur_program_id / 2;
            uint8_t bank_ab = cur_program_id % 2;

            screen::set_cursor(0, 1);
            screen::write(cur_profile.title()[0]);
            screen::write_pgm(PSTR(" BANK"), 5);
            screen::print_num(2, bank_no, '0');
            screen::write(' ');
            screen::write('A' + bank_ab);
            screen::write(" [", 2);
            screen::print_num(3, cur_program_id, '0');
            screen::write(']');
            break;
        }
        case PROFILE_ROLAND_BD1: {
            uint8_t patch = cur_program_id + 1;
            uint8_t p_size = 1;
            if (patch > 9) p_size += 1;
            if (patch > 99) p_size += 1;

            uint8_t f_title_size = cur_profile.title_size();
            if (f_title_size > 16 - 7 - p_size) {
                f_title_size = 16 - 7 - p_size;
            }

            screen::set_cursor(0, 1);
            screen::write(cur_profile.title(), f_title_size);
            screen::fill(16 - 7 - p_size);
            screen::write_pgm(PSTR(" Patch "), 7);
            screen::print_num(p_size, patch, ' ');
            break;
        }
        case PROFILE_ADA_MP1: {
            uint8_t preset = cur_program_id + 1;
            uint8_t p_size = 1;
            if (preset > 9) p_size += 1;
            if (preset > 99) p_size += 1;

            uint8_t f_title_size = cur_profile.title_size();
            if (f_title_size > 16 - 8 - p_size) {
                f_title_size = 16 - 8 - p_size;
            }

            screen::set_cursor(0, 1);
            screen::write(cur_profile.title(), f_title_size);
            screen::fill(16 - 8 - p_size);
            screen::write_pgm(PSTR(" Preset "), 8);
            screen::print_num(p_size, preset, ' ');
            break;
        }
        case PROFILE_CUSTOM:
            screen::set_cursor(0, 1);
            screen::write(cur_profile.title());
            break;
        }
        screen::fill();
    } else {
        screen::set_cursor(0, 1);
        screen::write(settings.title);
    }
}

void show_profile(uint8_t profile_id) {
    if (profile_id >= MAX_PROFILE) return;

    profile_t pf;

    if (profile_id == cur_profile_id) {
        pf = cur_profile;
    } else {
        storage::read(profile_id, pf);
    }

    set_title(8, pf.title(), pf.title_size());

    uint8_t c = pf.channel + settings.channel_start;
    uint8_t c_size = 1;
    if (c > 9) c_size += 1;

    screen::set_cursor(8, 1);
    screen::print_num(c_size, c, ' ');
    screen::fill();
}

void set_program(uint8_t p, uint8_t s, bool show_banner) {
    if (p >= MAX_PROGRAM) return;

    cur_program_id = p;
    if (s >= MAX_PROGRAM) {
        s = storage::program_seq(cur_profile_id, p);
    }
    cur_program_seq = s;

    storage::read(cur_profile_id, p, cur_program);

    if (cur_mode == MODE_PROGRAM_SELECT) {
        show_program(cur_program_id, cur_program_seq, show_banner);
    }

    stimer::schedule_in(STIMER_DELAY_MIDI_PROGRAM, settings.midi_send_delay_ms);
    stimer::schedule_in(STIMER_DELAY_STORE_PROGRAM, settings.store_program_delay_ms);
}

void midi_program() {
    uint8_t p = cur_program_id;

    switch (cur_profile_id) {
    case PROFILE_MOBIUS: {
        uint8_t cc0 = p < 128 ? 0 : 1;
        p = p < 128 ? p : p - 128;

        midi_cmd_t cmd0(cur_profile.channel, CMD_CTRL_CHANGE, 0, cc0);
        midi_cmd_t cmd1(cur_profile.channel, CMD_PROG_CHANGE, p);

        Serial1.write(&cmd0, cmd0.size());
        Serial1.write(&cmd1, cmd1.size());

        break;
    }
    case PROFILE_ROLAND_BD1: {
        uint8_t cc0 = p < 128 ? 80 : 81;
        p = p < 128 ? p : p - 128;

        midi_cmd_t cmd0(cur_profile.channel, CMD_CTRL_CHANGE, 0, cc0);
        midi_cmd_t cmd1(cur_profile.channel, CMD_CTRL_CHANGE, 32, 0);
        midi_cmd_t cmd2(cur_profile.channel, CMD_PROG_CHANGE, p);

        Serial1.write(&cmd0, cmd0.size());
        Serial1.write(&cmd1, cmd1.size());
        Serial1.write(&cmd2, cmd2.size());

        break;
    }
    case PROFILE_ADA_MP1: {
        midi_cmd_t cmd(cur_profile.channel, CMD_PROG_CHANGE, p);

        Serial1.write(&cmd, cmd.size());

        break;
    }
    }

    if (Serial.dtr()) {
        Serial.print("PC ");
        Serial.print(cur_profile_id);
        Serial.print(" ");
        Serial.println(cur_program_id);

        led::blink_up();
    }
}

void store_program() {
    if (settings.active_profile != cur_profile_id) {
        settings.active_profile = cur_profile_id;
        storage::write(settings, settings.active_profile);
    }

    if (cur_profile.active_program != cur_program_id) {
        cur_profile.active_program = cur_program_id;
        storage::write(cur_profile_id, cur_profile, cur_profile.active_program);
    }
}

void encoder::on_rotate(int8_t d) {
    current_mode()->on_rotate(d);
}

void keyboard::on_up_press() {
    uint8_t m = cur_mode;
    do {
        m = (m + 1) % MODE_MAX;
    } while (m == MODE_INIT);

    set_mode(m);
}

void keyboard::on_down_press() {
    uint8_t m = cur_mode;
    do {
        m = (m + MODE_MAX - 1) % MODE_MAX;
    } while (m == MODE_INIT);

    set_mode(m);
}

void keyboard::on_ok_press() {
    current_mode()->on_ok_press();
}

void keyboard::on_ok_hold() {
    current_mode()->on_ok_hold();
}

void update_clock() {
}

void update_display() {
    if (Serial.dtr() && display_1602::dirty) {
        Serial.print("D 0 ");
        Serial.write(screen::buf, sizeof(screen::buf));
        Serial.println("");
        display_1602::dirty = false;

        led::blink_up();
    }
}

void serial_print(uint8_t v) {
    Serial.print(" ");
    Serial.print(v);
}

void serial_print(int v) {
    Serial.print(" ");
    Serial.print(v);
}

void serial_print(const char *v) {
    Serial.print(" \"");
    Serial.print(v);
    Serial.print("\"");
}

template<typename T>
void serial_print(const prog_var_t<T> v) {
    T tmp;
    v.get(tmp);
    serial_print(tmp);
}

template<typename T, typename ...Args>
void serial_print(const T& v, Args... args) {
    serial_print(v);
    serial_print(args...);
}

void serial_reply_start(bool status, const serial_cmd_t &cmd) {
    if (status) {
        Serial.write(cmd.buf_ + cmd.arg_[0].start, cmd.arg_[0].size());
    } else {
        Serial.print("err:");
        Serial.write(cmd.buf_, cmd.size_);
    }
    led::blink_up();
}

void serial_reply(bool status, const serial_cmd_t &cmd) {
    if (Serial.dtr()) {
        serial_reply_start(status, cmd);
        Serial.println();
    }
}

template<typename ...Args>
void serial_reply(bool status, const serial_cmd_t &cmd, Args... args) {
    if (Serial.dtr()) {
        serial_reply_start(status, cmd);
        serial_print(args...);
        Serial.println();
    }
}

void process_cmd(const serial_cmd_t &cmd, bool output_reply) {
    if (cmd.command() == SCMD_PROGRAM_CHANGE) {
        uint8_t f = cur_profile_id;
        uint8_t p = cur_program_id;

        if (cmd.get_arg(1, f)) {
            if (f >= MAX_PROFILE) {
                if (output_reply) serial_reply(false, cmd);
                return;
            }

            profile_t pf;
            if (f == cur_profile_id) {
                pf = cur_profile;
            } else {
                storage::read(f, pf);
                p = pf.active_program;
            }

            if (cmd.get_arg(2, p)) {
                if (p >= MAX_PROGRAM) {
                    if (output_reply) serial_reply(false, cmd);
                    return;
                }

                if (f != cur_profile_id) {
                    cur_profile_id = f;
                    cur_profile = pf;
                }

                set_program(p);
            }
        }

        if (output_reply) serial_reply(true, cmd, f, p);

        return;
    }

    if (cmd.command() == SCMD_PROGRAM) {
        program_t pr = cur_program;
        uint8_t f = cur_profile_id;
        uint8_t p = cur_program_id;
        uint8_t s = MAX_PROGRAM;

        if (cmd.get_arg(1, f)) {
            if (f >= MAX_PROFILE) {
                if (output_reply) serial_reply(false, cmd);
                return;
            }

            if (cmd.get_arg(2, p)) {
                if (p >= MAX_PROGRAM) {
                    if (output_reply) serial_reply(false, cmd);
                    return;
                }

                if (f != cur_profile_id || p != cur_program_id) {
                    storage::read(f, p, pr);
                }

                if (cmd.get_arg(3, pr.data)) {
                    storage::write(f, p, pr);

                    if (cmd.get_arg(4, s)) {
                        if (s < MAX_PROGRAM) {
                            storage::program_seq(f, p, s);
                        }
                    }

                    if (f == cur_profile_id && p == cur_program_id) {
                        set_program(p);
                    }
                }
            }
        }

        if (output_reply && Serial.dtr()) {
            if (s >= MAX_PROGRAM) {
                s = storage::program_seq(f, p);
            }

            serial_reply(true, cmd, f, p, pr.title(), s);
        }

        return;
    }

    if (cmd.command() == SCMD_DISPLAY_DUMP) {
        uint16_t update_period = 0;
        bool output_period = false;

        if (cmd.get_arg(1, update_period)) {
            output_period = true;
        }

        if (update_period) {
            stimer::schedule_in(STIMER_DISPLAY_UPDATE, update_period, update_period);
        } else {
            stimer::cancel(STIMER_DISPLAY_UPDATE);
        }

        if (output_reply && Serial.dtr()) {
            Serial.print("D ");
            if (output_period) {
                Serial.print(update_period);
                Serial.print(" ");
            }
            Serial.write(screen::buf, sizeof(screen::buf));
            Serial.println();
            display_1602::dirty = false;

            led::blink_up();
        }

        return;
    }

    if (cmd.command() == SCMD_PROFILE) {
        profile_t pf = cur_profile;
        uint8_t f = cur_profile_id;

        if (cmd.get_arg(1, f)) {
            if (f >= MAX_PROFILE) {
                if (output_reply) serial_reply(false, cmd);
                return;
            }

            if (f != cur_profile_id) {
                storage::read(f, pf);
            }

            if (cmd.get_arg(2, pf.data)) {
                cmd.get_arg(3, pf.channel);
                cmd.get_arg(4, pf.port_mask);

                storage::write(f, pf);

                if (f == cur_profile_id) {
                    cur_profile = pf;
                }
            }
        }

        if (output_reply) {
            // serial_reply(true, cmd, f, pf.title(), pf.channel, pf.port_mask);
            serial_reply(true, cmd, f, pf.title(), pf.channel, pf.port_mask, (uint8_t) MAX_PROGRAM);
        }

        return;
    }

    if (cmd.command() == SCMD_PROGRAM_START) {
        uint8_t s = settings.program_start;

        if (cmd.get_arg(1, s)) {
            if (s > 1) {
                if (output_reply) serial_reply(false, cmd);
                return;
            }

            if (s != settings.program_start) {
                settings.program_start = s;

                storage::write(settings, settings.program_start);

                if (cur_mode == MODE_PROGRAM_SELECT) {
                    show_program(cur_program_id, cur_program_seq, false);
                }
            }
        }

        if (output_reply) {
            serial_reply(true, cmd, s);
        }

        return;
    }

    if (cmd.command() == SCMD_CHANNEL_START) {
        uint8_t s = settings.channel_start;

        if (cmd.get_arg(1, s)) {
            if (s > 1) {
                if (output_reply) serial_reply(false, cmd);
                return;
            }

            if (s != settings.channel_start) {
                settings.channel_start = s;

                storage::write(settings, settings.channel_start);
            }
        }

        if (output_reply) {
            serial_reply(true, cmd, s);
        }

        return;
    }

    if (cmd.command() == SCMD_MIDI_CHANNEL) {
        uint8_t s = settings.midi_channel_in;

        if (cmd.get_arg(1, s)) {
            if (s > 15) {
                if (output_reply) serial_reply(false, cmd);
                return;
            }

            if (s != settings.midi_channel_in) {
                settings.midi_channel_in = s;

                storage::write(settings, settings.midi_channel_in);
            }
        }

        if (output_reply) {
            serial_reply(true, cmd, s);
        }

        return;
    }

    if (cmd.command() == SCMD_DEVICE_TITLE) {
        if (cmd.get_arg(1, settings.title)) {
            storage::write(settings, settings.title);
        }

        if (output_reply) {
            serial_reply(true, cmd, (const char *) settings.title);
        }

        return;
    }

    if (cmd.command() == SCMD_FACTORY_DATA) {
        uint16_t len;
        if (cmd.get_arg(1, len)) {
            storage::write_factory_data(len);
        } else {
            storage::read_factory_data();
        }

        return;
    }

    if (cmd.command() == SCMD_BTN_HOLD) {
        set_mode(MODE_SETTINGS_FACTORY_RESET);
        current_mode()->on_ok_hold();
        if (output_reply) {
            serial_reply(true, cmd);
        }
        return;
    }

    if (cmd.command() == SCMD_VERSION) {
        if (output_reply) {
            serial_reply(true, cmd,
                version_t::build_date(), version_t::build_time(),
                version_t::fw_version(), version_t::hw_version(),
                version_t::serial_no());
        }
        return;
    }
}

serial_cmd_t serial_cmd;
bool serial_dtr;

}

void setup() {
    stimer::setup();

    keyboard::setup();
    encoder::setup();
    led::setup();

    spi::init();

    storage::setup();
    screen::setup();

    mode_init();

    stimer::schedule_in(STIMER_CLOCK, 0, CLOCK_PERIOD, update_clock);

    Serial1.begin(31250);
    Serial.begin(115200);

    serial_dtr = Serial.dtr();

    stimer::callback(STIMER_DISPLAY_UPDATE, update_display);
    stimer::callback(STIMER_DELAY_MIDI_PROGRAM, midi_program);
    stimer::callback(STIMER_DELAY_STORE_PROGRAM, store_program);

    if (storage::check()) {
        cur_mode = MODE_INIT;
        storage::read(settings);
        cur_profile_id = settings.active_profile;

        storage::read(cur_profile_id, cur_profile);

        set_program(cur_profile.active_program, MAX_PROGRAM, true);
        set_mode(MODE_PROGRAM_SELECT);
    } else {
        set_mode(MODE_SETTINGS_FACTORY_RESET);
    }
}

void loop() {
    clk_t::clock_source::update();

    stimer::update();

    keyboard::update();
    encoder::update();

    if (serial_dtr != Serial.dtr()) {
        serial_dtr = Serial.dtr();
        if (!serial_dtr) {
            stimer::cancel(STIMER_DISPLAY_UPDATE);
        }
    }

    while (Serial.available()) {
        int c = Serial.read();
        serial_cmd.read(c);

        led::blink_down();

        if (settings.echo) {
            Serial.write(c);
        }

        if (serial_cmd) {
            process_cmd(serial_cmd, true);

            serial_cmd.reset();

            return;
        }
    }

    artl::yield();
}
