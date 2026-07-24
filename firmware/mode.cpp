#include "mode.h"
#include "screen.h"
#include "led.h"
#include "storage.h"
#include "current_state.h"
#include "config.h"
#include "scroll_title.h"

#include <new>

namespace midi_controller {

struct settings_mode_t: public mode_t {
    void enter() const override {
        screen::set_cursor(0, 0);
        screen::write_pgm(PSTR("Settings        "));
        screen::set_cursor(0, 1);
    }
};

struct settings_midi_channel_mode_t: public settings_mode_t {
    void enter() const override {
        settings_mode_t::enter();
        screen::write_pgm(PSTR("MIDI Channel "));
        screen::print_num(2, settings.midi_channel_in, ' ');
        screen::fill();
    }
};

struct settings_program_start_mode_t: public settings_mode_t {
    void enter() const override {
        settings_mode_t::enter();
        screen::write_pgm(PSTR("Program Start "));
        screen::print_num(1, settings.program_start, ' ');
        screen::fill();
    }
};

struct settings_channel_start_mode_t: public settings_mode_t {
    void enter() const override {
        settings_mode_t::enter();
        screen::write_pgm(PSTR("Channel Start "));
        screen::print_num(1, settings.channel_start, ' ');
        screen::fill();
    }
};

struct settings_factory_reset_mode_t: public settings_mode_t {
    void enter() const override {
        settings_mode_t::enter();
        screen::write_pgm(PSTR("Factory Reset   "));

        led::start_blink_ok();
    }

    void leave() const override {
        led::stop_blink_ok();
    }

    void on_ok_hold() const override {
        screen::set_cursor(0, 0);
        screen::write_pgm(PSTR("Please wait...  "));
        storage::reset();

        storage::read(settings);
        cur_profile_id = settings.active_profile;

        storage::read(cur_profile_id, cur_profile);

        set_program(cur_profile.active_program, MAX_PROGRAM, true);

        set_mode(MODE_PROGRAM_SELECT);
    }
};

struct profile_select_mode_t: public mode_t {
    void enter() const override {
        screen::set_cursor(0, 0);
        screen::write_pgm(PSTR("Profile "));
        screen::set_cursor(0, 1);
        screen::write_pgm(PSTR("Channel "));

        select_profile_id = cur_profile_id;
        show_profile(select_profile_id);
    }

    void leave() const override {
        led::ok(false);
    }

    void on_ok_press() const override {
        if (select_profile_id != cur_profile_id) {
            cur_profile_id = select_profile_id;
            storage::read(cur_profile_id, cur_profile);

            set_program(cur_profile.active_program);
        }

        set_mode(MODE_PROGRAM_SELECT);
    }

    void on_rotate(int8_t d) const override {
        uint8_t p = (select_profile_id + MAX_PROFILE + d) % MAX_PROFILE;

        select_profile_id = p;
        show_profile(p);

        led::ok(select_profile_id != cur_profile_id);
    }
};

struct program_select_mode_t: public mode_t {
    void enter() const override {
        show_program(cur_program_id, cur_program_seq,
            cur_mode == MODE_INIT || cur_mode == MODE_SETTINGS_FACTORY_RESET);
    }

    void on_rotate(int8_t d) const override {
        uint8_t p = cur_program_id;
        uint8_t s = cur_program_seq;

        while (d != 0) {
            if (d < 0) {
                p = storage::read_prev(cur_profile_id, p);
                if (p != MAX_PROGRAM) {
                    --s;
                    d += 1;
                } else {
                    s = MAX_PROGRAM;
                }
            } else {
                p = storage::read_next(cur_profile_id, p);
                if (p != MAX_PROGRAM) {
                    ++s;
                    d -= 1;
                } else {
                    s = 255;
                }
            }
        }

        set_program(p, s);
    }
};

namespace {

mode_t mode[MODE_MAX];

}

uint8_t cur_mode;

void mode_init() {
    new (mode + MODE_PROGRAM_SELECT) program_select_mode_t();
    new (mode + MODE_PROFILE_SELECT) profile_select_mode_t();
    new (mode + MODE_SETTINGS_CHANNEL) settings_midi_channel_mode_t();
    new (mode + MODE_SETTINGS_PROGRAM_START) settings_program_start_mode_t();
    new (mode + MODE_SETTINGS_CHANNEL_START) settings_channel_start_mode_t();
    new (mode + MODE_SETTINGS_FACTORY_RESET) settings_factory_reset_mode_t();
}

void set_mode(uint8_t m) {
    if (m == cur_mode) {
        return;
    }

    if (m >= MODE_MAX) {
        return;
    }

    mode_t *ml = &mode[cur_mode];
    mode_t *me = &mode[m];

    ml->leave();
    scroll_title_stop();

    me->enter();

    cur_mode = m;
}

const mode_t *current_mode() {
    return mode + cur_mode;
}

}
