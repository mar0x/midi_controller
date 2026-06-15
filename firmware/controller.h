
#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include "config.h"
#include "debug.h"

#include "print_number.h"
#include "rotate.h"

#if defined(ARDUINO)

#include "eeprom.h"

#if defined(ARDUINO_AVR_PATCHMATE_X)

#include "spi_eeprom.h"
#include "spi_oled.h"

#include "in.h"
#include "out.h"

#else

#include <LiquidCrystal.h>

#if defined(ARDUINO_AVR_PATCHMATE)

#include "patchmate/in.h"
#include "patchmate/out.h"

#else

#include "dummy/in.h"
#include "dummy/out.h"

#endif

#endif

#else

#include "mock/spi_eeprom.h"
#include "mock/serial_pts.h"
#include "mock/time.h"
#include "mock/eeprom.h"
#include "mock/liquid_crystal.h"
#include "mock/in.h"
#include "mock/out.h"

#endif

#include "storage.h"

#include "artl/button.h"
#include "artl/encoder.h"
#include "artl/timer.h"
#include "artl/tc.h"

#include "midi_cmd.h"
#include "serial_cmd.h"
#include "settings.h"
#include "program.h"

#include "artl/yield.h"

#include "lcd.h"
#include "midi_mon.h"
#include "edit_value.h"
#include "version.h"
#include "serial_num.h"
#include "hardware_id.h"

namespace patch_mate {

struct controller_t {

    enum {
        MIN_CURSOR_POS = 4,
        MAX_CURSOR_POS = 16,
    };

    controller_t();

    void setup();
    void loop();

    bool check_storage();
    void reset_storage();

    void process_midi_cmd(const midi_cmd_t& cmd);
    void process_serial_cmd();
    void print_program(uint8_t p, const program_t& pr);
    template<typename T>
    void update_setting(uint8_t mode, T& s, T v);
    void midi_out(uint8_t b);
    void midi_out_cmd(const midi_cmd_t& cmd);

    void set_program(uint8_t prog, bool banner = false);
    void set_loop(uint8_t loop, bool val);
    void send_loop(uint8_t loop, bool val);

    bool led4loop() const;

    void on_down(uint8_t, bool);
    void on_hold(uint8_t);
    void on_rotate(short);

    void mute();
    void on_mute_timer();

    void store_blink_start(uint8_t count = 3, uint16_t period = 100);
    void on_store_blink_timer();

    void data_blink();
    void on_data_blink_timer();

    void commit();

    template<typename T>
    void start_edit(T& value, unsigned int min, unsigned int max,
        unsigned int step = 1, unsigned int display_offset = 0,
        unsigned int off = (unsigned int) -1,
        unsigned int on = (unsigned int) -1);
    void commit_edit();
    void set_edit_loop(uint8_t edit_loop);
    void set_mode(uint8_t m);

    void show_cursor();
    void hide_cursor();

    void show_title_hint();
    void hide_hint();

    void set_dirty(bool dirty);

    void show_master(bool master);

    void midi_dump_send();
    void midi_dump_start_subject(uint8_t subj, uint8_t size);
    void midi_dump_end_subject();
    void midi_dump(const void *d, uint8_t size);
    void midi_dump(uint8_t b);
    void midi_dump_flush();
    void midi_dump_read(uint8_t b);
    void midi_dump_read_stop(const char *reason, uint8_t size, uint8_t b);

    struct button_handler : public artl::default_button_handler {
        button_handler(uint8_t id) : id_(id) { }

        void on_down(bool, unsigned long);
        void on_hold(unsigned long);

        uint8_t id_;
    };

    struct encoder_handler : public artl::default_encoder_handler {
        void on_rotate(short, unsigned long);
    };

    struct in_traits {
        using button_handler = controller_t::button_handler;
        using encoder_handler = controller_t::encoder_handler;

        enum {
            LOOP0 = 0,
            STORE = 10,
            LEFT,
            RIGHT,
        };
    };

    enum {
        STORE = in_traits::STORE,
        LEFT = in_traits::LEFT,
        RIGHT = in_traits::RIGHT,
    };


    struct store_blink_timer_callback {
        void operator()(unsigned long) { }

        void start(uint8_t count, uint16_t period);
        void stop();

        uint8_t rest_;
        bool led_on_;
        uint16_t period_;
    };

    using store_blink_timer = artl::timer<store_blink_timer_callback>;


    enum {
        MODE_INITIAL,
        MODE_NORMAL,
        MODE_SETTINGS_FIRST,
        MODE_SETTINGS_CHANNEL = MODE_SETTINGS_FIRST,
        MODE_SETTINGS_CTRL_IN,
        MODE_SETTINGS_CTRL_OUT,
        MODE_SETTINGS_PROG_OUT,
        MODE_SETTINGS_MIDI_FWD,
        MODE_SETTINGS_MUTE_DELAY,
        MODE_SETTINGS_HIDE_CURSOR,
        MODE_SETTINGS_PROG_START,
        MODE_SETTINGS_CHANNEL_START,
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

    in<in_traits> in_;
    out out_;

    artl::timer<> mute_timer_;
    bool on_mute_ = true;
    bool wait_mute_ = false;

    store_blink_timer store_blink_timer_;
    artl::timer<> data_blink_timer_;
    unsigned long last_data_blink_ = 0;

    uint8_t mode_ = MODE_INITIAL;
    settings_t settings_;

    uint8_t edit_loop_;
    uint8_t edit_settings_offset_;
    edit_value_t edit_;

    enum {
        DUMP_CSUM_START = 0xFF,

        DUMP_SUBJECT = 0xE0,
        DUMP_SIZE = 0x1F,

        DUMP_NONE = 0x00,
        DUMP_SETTINGS = 0x20,
        DUMP_PROGRAM = 0x40,
        DUMP_MAGIC = 0xE0,
    };

    midi_cmd_t midi_cmd_;
    midi_cmd_t dump_cmd_;
    uint8_t dump_csum_;
    bool dump_read_started_;
    uint8_t dump_8th_bit_;
    uint8_t dump_subject_;
    uint8_t dump_pos_;
    uint8_t *dump_buf_;

    serial_cmd_t serial_cmd_;
    bool serial_connected_ = false;

    bool dirty_ = false;
    bool bad_magic_;

    uint8_t prog_id_ = 0;

    program_t program_;
    program_t program_master_;

    uint8_t cursor_pos_ = MIN_CURSOR_POS;
    artl::timer<> hide_cursor_timer_;

    unsigned long uptime_;

    midi_mon_t midi_mon_[2];

    volatile bool enable_input_;
    using input_tc = artl::tc<1>;

    bool show_master_;

    version_t version_;
    unsigned long t;
};

controller_t controller_;

#if defined(ISR) && defined(PCINT0_vect)
ISR(PCINT0_vect) {
}
#endif

#if defined(ISR) && defined(TIMER1_COMPA_vect)
ISR(TIMER1_COMPA_vect)
{
    controller_.enable_input_ = true;
}
#endif

inline
controller_t::controller_t()
{
}

inline void
controller_t::setup() {
    in_.setup();
    out_.setup();

    storage::setup();

    t = millis();

    midi_mon_[0].setup(t);
    midi_mon_[1].setup(t);

    lcd_setup();

    out_.mute();
    on_mute_ = true;

    out_.data_led(true);

    if (check_storage()) {
        set_mode(MODE_NORMAL);
        set_program(0, true);
    } else {
        set_mode(MODE_SETTINGS_FACTORY_RESET);
    }

    mute();

    enable_input_ = true;

#if defined(ARDUINO_ARCH_AVR)
    input_tc().setup(0, 0, 4, input_tc::cs::presc_256);
    input_tc().ocra() = 200;
    input_tc().cnt() = 0;
    input_tc().oca().enable();
#endif

    artl::yield();
}

inline void
controller_t::loop() {
    t = millis();

    while (!midi_cmd_.ready() && in_.midi_available()) {
        uint8_t b = in_.midi_read();

        debug(6, "midi read:", b);

        if (b != CMD_SYS_ACTIVE_S) {
            data_blink();
        }

        if (midi_mon_[0].active()) {
            midi_mon_[0].read(b);
        }

        if (settings_.midi_fwd) {
            midi_out(b);
        }

        if (is_midi_rt(b)) {
            out_.usb_midi_write(b, 1);
        } else {
            midi_cmd_ << b;
        }
    }

    if (midi_cmd_.ready()) {
        if (mode_ != MODE_MIDI_DUMP_RECV || !midi_cmd_.sys_ex()) {
            out_.usb_midi_write(midi_cmd_, 1);
        }

        process_midi_cmd(midi_cmd_);

        midi_cmd_.reset();
    }

    {
        midi_cmd_t cmd;
        uint8_t jack;

        if (in_.usb_midi_read(cmd, jack)) {
            if (cmd[0] != CMD_SYS_ACTIVE_S) {
                data_blink();
            }

            switch (jack) {
            case 0:
                process_midi_cmd(cmd);
                break;
            case 1:
                midi_out_cmd(cmd);
                break;
            }
        }
    }

    if (serial_connected_ != in_.serial_connected()) {
        serial_connected_ = in_.serial_connected();

        if (serial_connected_) {
            out_.serial_println("");
            out_.serial_print(version_.sn(), "> ");
        }
    }

    while (!serial_cmd_ && in_.serial_available()) {
        uint8_t b = in_.serial_read();

        data_blink();

        serial_cmd_.read(b);

        if (settings_.echo) {
            // Serial echo
            out_.serial_print((char) b);
        }
    }

    if (serial_cmd_) {
        process_serial_cmd();
    }

    in_.update(t, enable_input_);

#if defined(ISR) && defined(TIMER1_COMPA_vect)
    if (!enable_input_) {
        artl::yield();
        return;
    }

    enable_input_ = false;
#endif

    if (mute_timer_.update(t)) {
        on_mute_timer();
    }

    if (hide_cursor_timer_.update(t)) {
        hide_hint();
        hide_cursor();

        lcd_update(0, 1, LCD_COLUMNS);
    }

    if (store_blink_timer_.update(t)) {
        on_store_blink_timer();
    }

    if (data_blink_timer_.update(t)) {
        on_data_blink_timer();
    }

    switch (mode_) {
    case MODE_MIDI_IN_MONITOR:
        midi_mon_[0].update(t);
        break;

    case MODE_MIDI_OUT_MONITOR:
        midi_mon_[1].update(t);
        break;

    case MODE_UPTIME: {
        unsigned long s = t / 1000;
        if (uptime_ < s) {
            uptime_ = s;
            print_number(lcd_buf[1], 12, uptime_);
            lcd_update(0, 1, 12);
        }
        break;
    }
    }

    if (!store_blink_timer_.active()) {
        out_.store_led(dirty_ && !show_master_);
    }

    if (out_.relay_changed()) {
        out_.commit_led();
        mute();
    } else {
        out_.commit();
    }

    artl::yield();
}

inline bool
controller_t::check_storage() {
    serial_num_t sn;
    storage::read(sn);
    version_.sn(sn);

    hardware_id_t hw;
    storage::read(hw);
    version_.hw(hw);

    if (!storage::check()) {
        bad_magic_ = true;
        return false;
    }

    storage::read(settings_);

    bad_magic_ = false;

#if defined(DEBUG)
    debug_level_ = settings_.usb_debug;
#endif
    return true;
}

inline void
controller_t::reset_storage() {
    storage::reset();

    check_storage();
}

inline void
controller_t::process_midi_cmd(const midi_cmd_t& cmd) {
    if (mode_ == MODE_MIDI_DUMP_RECV) {
        if (!dump_read_started_ && cmd.command() == CMD_SYS_EX && cmd[1] == eemagic[0]) {
            memcpy(lcd_buf[1], "  reading...    ", 16);
            lcd_update(0, 1, LCD_COLUMNS);

            dump_read_started_ = true;
            dump_8th_bit_ = 0;
            dump_subject_ = DUMP_MAGIC | sizeof(eemagic);
            dump_csum_ = DUMP_CSUM_START;
            dump_pos_ = 1;

            if (midi_mon_[0].active()) {
                midi_mon_[0].toggle_pause(t);
            }

            dump_buf_ = midi_mon_[0].data();

            if (cmd.size() == 3) {
                midi_dump_read(cmd[2]);
            }

            return;
        }
        if (dump_read_started_ && cmd.sys_ex()) {
            for (uint8_t i = 0; i < cmd.size() && dump_read_started_; ++i) {
                midi_dump_read(cmd[i]);
            }
            return;
        }
        if (dump_read_started_) {
            midi_dump_read(cmd[0]);
        }
    }

    if (cmd.command() == CMD_PROG_CHANGE &&
        cmd.channel() == settings_.midi_channel) {
        set_program(cmd.program());
        return;
    }

    if (cmd.command() == CMD_CTRL_CHANGE &&
        cmd.channel() == settings_.midi_channel) {
        for(uint8_t i = 0; i < MAX_LOOP; i++) {
            if (settings_.midi_loop_ctrl_in[i] == cmd.controller()) {
                set_loop(i, cmd.value() != 0);
            }
        }
        return;
    }
}

inline void
controller_t::process_serial_cmd() {
    out_.serial_println("");

    bool old_dirty = dirty_;

    // out_.serial_println(serial_cmd_.buf_);

    switch (serial_cmd_.command()) {
    case serial_cmd_t::CMD_VERSION:
        for (uint8_t i = 0; i < version_t::MAX_LINE; ++i) {
            out_.serial_println(version_.lines[i]);
        }
        break;

    case serial_cmd_t::CMD_HELP: {
        for (uint16_t i = 0; i < sizeof(help_); ++i) {
            char b = pgm_read_byte_near(help_ + i);
            if (b == '\n') { out_.serial_print('\r'); }
            out_.serial_print(b);
        }
        break;
    }
    case serial_cmd_t::CMD_PROG_CHANGE: {
        uint8_t p;
        if (serial_cmd_.get_arg(1, p) && p < MAX_PROGRAMS) { set_program(p); }

        out_.serial_println("PC ", prog_id_);
        break;
    }
    case serial_cmd_t::CMD_CTRL_CHANGE: {
        uint8_t c = 0, v = 0;
        if (serial_cmd_.get_arg(1, c) && c < MAX_CTRL &&
            serial_cmd_.get_arg(2, v)) {
            for(uint8_t i = 0; i < MAX_LOOP; i++) {
                if (settings_.midi_loop_ctrl_in[i] == c) {
                    set_loop(i, v != 0);
                }
            }
        }

        out_.serial_println("CC ", c, " ", v);
        break;
    }
    case serial_cmd_t::CMD_LOOP: {
        uint8_t l = 0;
        uint8_t v = 0;
        if (serial_cmd_.get_arg(1, l) && l < MAX_LOOP &&
            serial_cmd_.get_arg(2, v) && (v == 0 || v == 1)) {
            set_loop(l, v);
        }

        if (l >= MAX_LOOP) l = 0;

        out_.serial_println("LP ", l, " ", program_.loop[l]);
        break;
    }
    case serial_cmd_t::CMD_NAME: {
        serial_cmd_.get_arg(1, program_.title);

        if (mode_ == MODE_NORMAL) {
            memcpy(lcd_buf[0] + 4, program_.title, 12);

            hide_hint();
            hide_cursor();

            lcd_update();
        }

        out_.serial_println("NM \"", program_.title, "\"");
        break;
    }
    case serial_cmd_t::CMD_MODE: {
        uint8_t m = mode_;
        if (serial_cmd_.get_arg(1, m) && m < MODE_MAX) { set_mode(m); }

        out_.serial_println("MD ", mode_);
        break;
    }
    case serial_cmd_t::CMD_STORE: {
        if (mode_ == MODE_SETTINGS_FACTORY_RESET) {
            reset_storage();
            set_mode(MODE_NORMAL);
            set_program(0, true);
            store_blink_start();

            out_.serial_println("FR");
            break;
        }

        if (mode_ == MODE_NORMAL) {
            if (!dirty_) { out_.serial_println("CLEAR"); }
            else if (bad_magic_) { out_.serial_println("BAD MAGIC"); }
            else { commit(); }

          print_program(prog_id_, program_);
        }

        break;
    }
    case serial_cmd_t::CMD_RESTORE: {
        if (!dirty_) { out_.serial_println("CLEAR"); }
        else { set_program(prog_id_); }

        print_program(prog_id_, program_);
        break;
    }
    case serial_cmd_t::CMD_PROGRAM: {
        program_t pr = program_;
        uint8_t p = prog_id_;
        loop_value_t l = pr.loop;

        if (serial_cmd_.get_arg(1, p)) {
            if (p >= MAX_PROGRAMS) p = prog_id_;

            if (p != prog_id_) {
                storage::read(p, pr);
            }

            if (serial_cmd_.get_arg(2, l) && !bad_magic_) {
                pr.loop = l;

                serial_cmd_.get_arg(3, pr.title);

                storage::write(p, pr);

                if (p == prog_id_) { set_program(prog_id_); }
            }
        }

        print_program(p, pr);
        break;
    }
    case serial_cmd_t::CMD_MIDI_CHANNEL: {
        uint8_t v = settings_.midi_channel;
        if (serial_cmd_.get_arg(1, v) && v < MAX_CHANNEL) {
            update_setting(MODE_SETTINGS_CHANNEL, settings_.midi_channel, v);
        }

        out_.serial_println("MC ", v);
        break;
    }
    case serial_cmd_t::CMD_MIDI_PROG_OUT: {
        uint8_t v = settings_.midi_out_prog;
        if (serial_cmd_.get_arg(1, v)) {
            v = v != 0;
            update_setting(MODE_SETTINGS_PROG_OUT, settings_.midi_out_prog, v);
        }

        out_.serial_println("MO ", v);
        break;
    }
    case serial_cmd_t::CMD_MIDI_FORWARD: {
        uint8_t v = settings_.midi_fwd;
        if (serial_cmd_.get_arg(1, v)) {
            v = v != 0;
            update_setting(MODE_SETTINGS_MIDI_FWD, settings_.midi_fwd, v);
        }

        out_.serial_println("MF ", v);
        break;
    }
#if defined(DEBUG)
    case serial_cmd_t::CMD_DEBUG_LEVEL: {
        uint8_t v = settings_.usb_debug;
        if (serial_cmd_.get_arg(1, v)) {
            if (v > 9) v = 9;
            update_setting(MODE_SETTINGS_USB_DEBUG, settings_.usb_debug, v);
            debug_level_ = v;
        }

        out_.serial_println("DL ", v);
        break;
    }
#endif
    case serial_cmd_t::CMD_MUTE_DELAY: {
        uint16_t v = settings_.mute_delay_ms;
        if (serial_cmd_.get_arg(1, v)) {
            update_setting(MODE_SETTINGS_HIDE_CURSOR, settings_.mute_delay_ms, v);
        }

        out_.serial_println("ML ", v);
        break;
    }
    case serial_cmd_t::CMD_HIDE_CURSOR_DELAY: {
        uint8_t v = settings_.hide_cursor_delay_s;
        if (serial_cmd_.get_arg(1, v)) {
            update_setting(MODE_SETTINGS_HIDE_CURSOR, settings_.hide_cursor_delay_s, v);
        }

        out_.serial_println("HC ", v);
        break;
    }
    case serial_cmd_t::CMD_FACTORY_RESET: {
        set_mode(MODE_SETTINGS_FACTORY_RESET);
        break;
    }
    case serial_cmd_t::CMD_MIDI_LOOP_IN_CTRL: {
        // TODO
        break;
    }
    case serial_cmd_t::CMD_MIDI_LOOP_OUT_CTRL: {
        // TODO
        break;
    }
    case serial_cmd_t::CMD_MIDI_MON_IN: {
        // TODO
        break;
    }
    case serial_cmd_t::CMD_MIDI_MON_OUT: {
        // TODO
        break;
    }
    case serial_cmd_t::CMD_MIDI_DUMP_SEND: {
        midi_dump_send();
        break;
    }
    case serial_cmd_t::CMD_MIDI_DUMP_RECV: {
        set_mode(MODE_MIDI_DUMP_RECV);
        break;
    }
    case serial_cmd_t::CMD_ECHO: {
        uint8_t v;
        if (serial_cmd_.get_arg(1, v)) {
            settings_.echo = v != 0;

            if (!bad_magic_) {
                storage::write(settings_, settings_.echo);
            }
        }

        out_.serial_println("E ", settings_.echo);
        break;
    }
    case serial_cmd_t::CMD_SERIAL_NUMBER: {
        struct {
            serial_num_t sn;
            char z = 0;
        } v;

        if (serial_cmd_.get_arg(1, v.sn)) {
            storage::write(v.sn);
            version_.sn(v.sn);
        } else {
            storage::read(v.sn);
        }

        out_.serial_println("SN ", v.sn.buf);
        break;
    }
    case serial_cmd_t::CMD_HARDWARE: {
        struct {
            hardware_id_t hw;
            char z = 0;
        } v;

        if (serial_cmd_.get_arg(1, v.hw)) {
            storage::write(v.hw);
            version_.hw(v.hw);
        } else {
            storage::read(v.hw);
        }

        out_.serial_println("HW ", v.hw.buf);
        break;
    }
    case serial_cmd_t::CMD_BTN_PRESS: {
        struct {
            char btn[1];
            char z = 0;
        } btn;

        if (serial_cmd_.get_arg(1, btn.btn)) {
            if (btn.btn[0] >= '0' && btn.btn[0] <= '9') {
                on_down(btn.btn[0] - '0', true);
            } else {
                switch(btn.btn[0]) {
                case 'L': on_down(LEFT, true); break;
                case 'R': on_down(RIGHT, true); break;
                case 'U': on_rotate(1); break;
                case 'D': on_rotate(-1); break;
                case 'S': on_hold(STORE); break;
                }
            }
        }

        out_.serial_println("B ", btn.btn);
        break;
    }
    case serial_cmd_t::CMD_DISPLAY_DUMP: {
        out_.serial_write((const uint8_t *) lcd_buf[0], LCD_COLUMNS);
        out_.serial_println("");
        out_.serial_write((const uint8_t *) lcd_buf[1], LCD_COLUMNS);
        out_.serial_println("");
        break;
    }
    }

    if (mode_ == MODE_NORMAL) {
        set_dirty(program_ != program_master_);

        if (old_dirty != dirty_) {
            out_.serial_println(dirty_ ? "DIRTY" : "CLEAR");
        }
    }

    serial_cmd_.reset();

    out_.serial_print("> ");
}

inline void
controller_t::print_program(uint8_t p, const program_t& pr) {
    out_.serial_print("PR ", p, " ", pr.loop[0]);
    for (uint8_t i = 1; i < MAX_LOOP; i++) {
        out_.serial_print(",", pr.loop[i]);
    }
    out_.serial_println(" \"", pr.title, "\"");
}

template<typename T>
inline void
controller_t::update_setting(uint8_t mode, T& s, T v) {
    if (mode_ == mode) {
        edit_.value(v);
        edit_.print(lcd_buf[1] + 13);

        set_dirty(false);

        lcd_update(13, 1, 3);
    }

    s = v;
    storage::write(settings_, s);
}

inline void
controller_t::midi_out(uint8_t b) {
    if (midi_mon_[1].active()) {
        midi_mon_[1].read(b);
    }

    out_.midi_write(b);

    if (b != CMD_SYS_ACTIVE_S) {
        data_blink();
    }
}

inline void
controller_t::midi_out_cmd(const midi_cmd_t& cmd) {
    for (uint8_t i = 0; i < cmd.size(); i++) {
        midi_out(cmd[i]);
    }
}

inline void
controller_t::set_program(uint8_t prog, bool banner) {
    debug(1, "set_program: ", prog_id_, " -> ", prog);

    if (prog >= MAX_PROGRAMS) return;

    prog_id_ = prog;

    storage::read(prog, program_);
    program_master_ = program_;

    if (settings_.midi_out_prog) {
        midi_cmd_t cmd(settings_.midi_channel, CMD_PROG_CHANGE, prog_id_);

        out_.usb_midi_write(cmd, 0);
        midi_out_cmd(cmd);
    }

    for(uint8_t i = 0; i < MAX_LOOP; i++) {
        out_.loop_relay(i, program_.loop[i]);
        send_loop(i, program_.loop[i]);
    }

    if (mode_ == MODE_NORMAL) {
        set_dirty(false);

        print_number(lcd_buf[0], 3, prog_id_ + settings_.prog_start);
        lcd_buf[0][3] = ' ';
        memcpy(lcd_buf[0] + 4, program_.title, 12);

        hide_hint();
        hide_cursor();

        if (banner) {
            memcpy(lcd_buf[1], "  PatchMate X   ", LCD_COLUMNS);
        }

        lcd_update();
    }

    // adjust loop led for all modes except MIDI mon and control settings
    if (led4loop()) {
        out_.loop_led(program_.loop);
    }
}

inline void
controller_t::set_loop(uint8_t i, bool v) {
    debug(1, "set_loop: ", i, " ", program_.loop[i], " -> ", v);

    out_.loop_relay(i, v);

    // send controller_t change MIDI command if configured
    send_loop(i, v);

    // adjust loop led for all modes except MIDI mon and control settings
    if (led4loop()) {
        out_.loop_led(i, v);
    }

    program_.loop.set(i, v);

    if (mode_ == MODE_NORMAL) {
        set_dirty(program_ != program_master_);
    }
}

inline void
controller_t::send_loop(uint8_t i, bool v) {
    if (settings_.midi_loop_ctrl_out[i] <= 127) {
        midi_cmd_t cmd(settings_.midi_channel, CMD_CTRL_CHANGE,
            settings_.midi_loop_ctrl_out[i], v ? 127 : 0);

        out_.usb_midi_write(cmd, 0);
        midi_out_cmd(cmd);
    }
}

inline bool
controller_t::led4loop() const {
    return (mode_ != MODE_SETTINGS_CTRL_IN) &&
           (mode_ != MODE_SETTINGS_CTRL_OUT) &&
           (mode_ != MODE_MIDI_IN_MONITOR) &&
           (mode_ != MODE_MIDI_OUT_MONITOR);
}

inline void
controller_t::on_down(uint8_t i, bool down) {
    if (!down) {
        debug(5, "on_down: release ", i);

        if (i == RIGHT && show_master_) {
            show_master(false);
        }

        return; // no reaction on key UP
    }

    debug(4, "on_down: ", i);

    // change mode: hold LEFT and press LOOP
    if (i < MAX_LOOP && in_.left().down()) {
        switch(i) {
            case 0: set_mode(MODE_SETTINGS_CHANNEL); break;
            case 1: set_mode(MODE_SETTINGS_CTRL_IN); break;
            case 2: set_mode(MODE_SETTINGS_CTRL_OUT); break;
            case 3: set_mode(MODE_SETTINGS_PROG_OUT); break;
            case 4: set_mode(MODE_SETTINGS_MIDI_FWD); break;
            case 5: set_mode(MODE_SETTINGS_MUTE_DELAY); break;
            case 6: set_mode(MODE_SETTINGS_HIDE_CURSOR); break;
            case 7: set_mode(MODE_SETTINGS_PROG_START); break;
            case 8: set_mode(MODE_SETTINGS_CHANNEL_START); break;
#if defined(DEBUG)
            case 9: set_mode(MODE_SETTINGS_USB_DEBUG); break;
#endif
        }
        return;
    }

    // change mode ex: hold RIGHT and press LOOP
    if (i < MAX_LOOP && in_.right().down()) {
        switch(i) {
            case 0: set_mode(MODE_PROGRAM_SWAP); break;
            case 1: set_mode(MODE_PROGRAM_MOVE); break;
            case 2: set_mode(MODE_MIDI_IN_MONITOR); break;
            case 3: set_mode(MODE_MIDI_OUT_MONITOR); break;
            case 4: set_mode(MODE_MIDI_DUMP_SEND); break;
            case 5: set_mode(MODE_MIDI_DUMP_RECV); break;
            case 9: set_mode(MODE_ABOUT); break;
        }
        return;
    }

    if (mode_ == MODE_NORMAL) {
        // toggle LOOP
        if (i < MAX_LOOP && in_.left().up() && in_.right().up()) {
            bool v = program_.loop[i];

            debug(2, "toggle loop ", i, " ", v, " -> ", !v);
            set_loop(i, !v);

            return;
        }

        // move cursor LEFT
        if (i == LEFT && in_.right().up()) {
            cursor_pos_ = rotate(cursor_pos_, MIN_CURSOR_POS, MAX_CURSOR_POS, -1);
            hide_hint();
            lcd_update(0, 1, LCD_COLUMNS);

            show_cursor();

            return;
        }

        // move cursor RIGHT
        if (i == RIGHT && in_.left().up()) {
            cursor_pos_ = rotate(cursor_pos_, MIN_CURSOR_POS, MAX_CURSOR_POS, 1);
            hide_hint();
            lcd_update(0, 1, LCD_COLUMNS);

            show_cursor();

            return;
        }

        // cancel changes and stay in NORMAL
        // hold RIGHT then press LEFT or
        // hold LEFT then press RIGHT
        if ((i == LEFT && in_.right().hold()) || (i == RIGHT && in_.left().hold()))
        {
            if (dirty_) { set_program(prog_id_); }
            return;
        }

        return;
    }

    // cancel changes and back to NORMAL mode
    // hold RIGHT then press LEFT or
    // hold LEFT then press RIGHT
    if ((i == LEFT && in_.right().hold()) || (i == RIGHT && in_.left().hold()))
    {
        set_mode(MODE_NORMAL);
        return;
    }

    if (mode_ == MODE_SETTINGS_CTRL_IN || mode_ == MODE_SETTINGS_CTRL_OUT) {
        bool loop_changed = false;

        // controller_t -> LOOP mapping
        if (i == LEFT && in_.right().up()) {
            set_edit_loop(rotate(edit_loop_, 1, MAX_LOOP + 1, -1));
            loop_changed = true;
        }

        if (i == RIGHT && in_.left().up()) {
            set_edit_loop(rotate(edit_loop_, 1, MAX_LOOP + 1, 1));
            loop_changed = true;
        }

        if (i < MAX_LOOP && in_.left().up() && in_.right().up()) {
            if (i + 1 != edit_loop_) {
                set_edit_loop(i + 1);
                loop_changed = true;
            }
        }

        if (loop_changed) {
            lcd_update(0, 1);
            lcd_update(13, 1, 3);
        }

        return;
    }

    if (mode_ == MODE_ABOUT) {
        if (i == LEFT && in_.right().up()) {
            version_.rotate(-1);
            version_.print_line();
            lcd_update(0, 1, LCD_COLUMNS);
        }

        if (i == RIGHT && in_.left().up()) {
            version_.rotate(1);
            version_.print_line();
            lcd_update(0, 1, LCD_COLUMNS);
        }

        return;
    }

    if (mode_ == MODE_MIDI_IN_MONITOR ||
        mode_ == MODE_MIDI_OUT_MONITOR) {
        uint8_t m = mode_ - MODE_MIDI_IN_MONITOR;
        if (i == STORE) {
            // enable/pause capturing
            set_dirty(midi_mon_[m].toggle_pause(t));
        }

        if (i == LEFT && in_.right().up()) {
            midi_mon_[m].mode_prev(t);

            out_.loop_led(1 << midi_mon_[m].mode());
        }

        if (i == RIGHT && in_.left().up()) {
            midi_mon_[m].mode_next(t);

            out_.loop_led(1 << midi_mon_[m].mode());
        }

        if (i < MAX_LOOP && i < midi_mon_t::MODE_MAX && in_.left().up() && in_.right().up()) {
            midi_mon_[m].mode(i, t);

            out_.loop_led(1 << i);
        }
    }
}

inline void
controller_t::on_hold(uint8_t i) {
    debug(4, "on_hold: ", i);

    if (i == STORE && in_.left().hold() && in_.right().hold()) {
        set_mode(MODE_SETTINGS_FACTORY_RESET);
        return;
    }

    if (mode_ == MODE_NORMAL) {
        // hold STORE
        if (i == STORE) {
            if (!dirty_) { return; }

            if (bad_magic_) {
                set_mode(MODE_SETTINGS_FACTORY_RESET);
                return;
            }

            commit();

            return;
        }

        if (i == RIGHT && in_.left().up()) {
            if (!dirty_) { return; }

            show_master(true);
        }

        if ((i == LEFT && in_.right().down()) || (i == RIGHT && in_.left().down())) {
            // cancel changes
            if (dirty_) { set_program(prog_id_); }
        }

        return;
    }

    if (i == STORE) {
        if (mode_ >= MODE_SETTINGS_FIRST &&
            mode_ <= MODE_SETTINGS_LAST) {
            // No changes made - switch back to NORMAL;
            if (!dirty_) {
                set_mode(MODE_NORMAL);
                return;
            }

            // Changes made, but storage is not ready - confirm storage Reset first;
            if (bad_magic_ && mode_ != MODE_SETTINGS_FACTORY_RESET) {
                set_mode(MODE_SETTINGS_FACTORY_RESET);
                return;
            }
        }

        switch (mode_) {
        case MODE_SETTINGS_CTRL_IN:
        case MODE_SETTINGS_CTRL_OUT:
            commit_edit();

            edit_.value(edit_.value());
            set_dirty(false);

            // stay in same mode for convenience
            store_blink_start();
            return;

        case MODE_SETTINGS_CHANNEL:
        case MODE_SETTINGS_PROG_OUT:
        case MODE_SETTINGS_MIDI_FWD:
        case MODE_SETTINGS_HIDE_CURSOR:
        case MODE_SETTINGS_PROG_START:
        case MODE_SETTINGS_CHANNEL_START:
            commit_edit();
            break;

        case MODE_SETTINGS_MUTE_DELAY:
            settings_.mute_delay_ms = edit_.value();
            storage::write(settings_, settings_.mute_delay_ms);
            break;

        case MODE_SETTINGS_FACTORY_RESET:
            reset_storage();
            set_mode(MODE_NORMAL);
            set_program(0, true);
            store_blink_start();
            return;

#if defined(DEBUG)
        case MODE_SETTINGS_USB_DEBUG:
            commit_edit();
            debug_level_ = edit_.value();
            break;
#endif

        case MODE_PROGRAM_SWAP:
        case MODE_PROGRAM_MOVE:
            // No changes made - switch back to NORMAL;
            if (!dirty_) {
                set_mode(MODE_NORMAL);
                return;
            }

            // Changes made, but storage not ready - confirm storage Reset first;
            if (bad_magic_ && mode_ != MODE_SETTINGS_FACTORY_RESET) {
                set_mode(MODE_SETTINGS_FACTORY_RESET);
                return;
            }

            {
                program_t tmp_prog;

                if (mode_ == MODE_PROGRAM_SWAP) {
                    storage::read(edit_.value(), tmp_prog);
                    storage::write(prog_id_, tmp_prog);
                } else {
                    int step = edit_.value() > prog_id_ ? 1 : -1;

                    for (uint8_t p = prog_id_; p != edit_.value(); p += step) {
                        storage::read(p + step, tmp_prog);
                        storage::write(p, tmp_prog);
                    }
                }

                storage::write(edit_.value(), program_master_);
                prog_id_ = edit_.value();
            }

            break;

        case MODE_MIDI_DUMP_SEND:
            midi_dump_send();
            break;
        }

        set_mode(MODE_NORMAL);
        store_blink_start();

        return;
    }

    // cancel changes and back to NORMAL mode
    // hold RIGHT then press LEFT or
    // hold LEFT then press RIGHT
    if ((i == LEFT && in_.right().down()) || (i == RIGHT && in_.left().down())) {
        set_mode(MODE_NORMAL);
    }
}

inline void
controller_t::commit() {
    // commit program change
    storage::write(prog_id_, program_);

    program_master_ = program_;

    store_blink_start();
    set_dirty(false);
}

/* Available alphabet:
  ABCDEFGHIJKLMNOPRSTUVWXYZ[\]^_`
  abcdefghijklmnoprstuvwxyz{|}~
  !"#$%&'()*+,-./
  :;<=>?@
  0123456789
*/

/*
joints = {
    { ' ', 'A' },
    { '~', '!' },
    { '/', ':' },
    { '@', '0' },
    { '9', ' ' }
};

ranges = {
    { ' ', ' ' },
    { 'A', '~' },
    { '!', '/' },
    { ':', '@' }
    { '0', '9' },
};
*/

inline char
rotate_char(char c, short dir) {
    if (dir > 0) {
        if (c == ' ') return 'A';
        if (c >= '0' && c < '9') return c + 1;
        if (c >= 'A' && c < '~') return c + 1;
        if (c >= '!' && c < '/') return c + 1;
        if (c >= ':' && c < '@') return c + 1;
        if (c == '_') return '!';
        if (c == '/') return ':';
        if (c == '@') return '0';
        if (c == '9') return ' ';
        return ' ';
    }

    if (c == ' ') return '9';
    if (c > '0' && c <= '9') return c - 1;
    if (c > 'A' && c <= '~') return c - 1;
    if (c > '!' && c <= '/') return c - 1;
    if (c > ':' && c <= '@') return c - 1;
    if (c == '0') return '@';
    if (c == ':') return '/';
    if (c == '!') return '_';
    if (c == 'A') return ' ';
    return ' ';
}

inline void
controller_t::on_rotate(short dir) {
    debug(4, "on_rotate: ", dir);

    // rotate on RIGHT hold - change mode
    if (in_.right().hold()) {
        uint8_t m = rotate(mode_, MODE_NORMAL, MODE_MAX, dir);

        set_mode(m);
        return;
    }

    switch(mode_) {
    case MODE_NORMAL: {
        // rotate on LEFT hold - change program
        if (in_.left().hold() || !hide_cursor_timer_.active()) {
            set_program(rotate(prog_id_, MAX_PROGRAMS, dir));

            return;
        }

        char& c = program_.title[cursor_pos_ - MIN_CURSOR_POS];
        c = rotate_char(c, dir);
        lcd_buf[0][cursor_pos_] = c;

        show_title_hint();

        lcd.setCursor(cursor_pos_, 0);
        lcd.write(c);
        show_cursor();

        set_dirty(program_ != program_master_);

        return;
    }

    case MODE_SETTINGS_CHANNEL:
    case MODE_SETTINGS_CTRL_IN:
    case MODE_SETTINGS_CTRL_OUT:
    case MODE_SETTINGS_PROG_OUT:
    case MODE_SETTINGS_MIDI_FWD:
    case MODE_SETTINGS_MUTE_DELAY:
    case MODE_SETTINGS_HIDE_CURSOR:
    case MODE_SETTINGS_PROG_START:
    case MODE_SETTINGS_CHANNEL_START:
#if defined(DEBUG)
    case MODE_SETTINGS_USB_DEBUG:
#endif
        edit_.rotate(dir);
        edit_.print(lcd_buf[1] + 13);

        set_dirty(edit_.dirty());

        lcd_update(13, 1, 3);
        break;

    case MODE_PROGRAM_SWAP:
    case MODE_PROGRAM_MOVE: {
        edit_.rotate(dir);
        set_dirty(edit_.dirty());

        program_t dprog;
        storage::read(edit_.value(), dprog);
        print_number(lcd_buf[1] + 3, 3, edit_.value() + settings_.prog_start);
        memcpy(lcd_buf[1] + 7, dprog.title, 9);

        lcd_update(3, 1, 13);
        break;
    }

    case MODE_MIDI_IN_MONITOR:
    case MODE_MIDI_OUT_MONITOR: {
        uint8_t m = mode_ - MODE_MIDI_IN_MONITOR;
        if (dir > 0) {
            midi_mon_[m].scroll_down(t);
        } else {
            midi_mon_[m].scroll_up(t);
        }
        break;
    }

    }
}


inline void
controller_t::mute() {
    if (wait_mute_) return;

    out_.mute();

    wait_mute_ = true;
    mute_timer_.schedule(t + (on_mute_ ? 0 : settings_.mute_delay_ms));
}

inline void
controller_t::on_mute_timer() {
    if (wait_mute_) {
        on_mute_ = true;

        out_.commit();

        wait_mute_ = false;
        mute_timer_.schedule(t + settings_.mute_delay_ms);
    } else {
        on_mute_ = false;
        out_.unmute();
    }
}

inline void
controller_t::data_blink() {
    if (data_blink_timer_.active() || (t - last_data_blink_) < 200) return;

    out_.data_led(false);

    data_blink_timer_.schedule(t + 50);
}

inline void
controller_t::on_data_blink_timer() {
    out_.data_led(true);

    last_data_blink_ = t;
}

template<typename T>
inline void
controller_t::start_edit(T& value,
    unsigned int min, unsigned int max,
    unsigned int step, unsigned int display_offset,
    unsigned int off, unsigned int on)
{
    edit_settings_offset_ = (uint8_t *) &value - (uint8_t *) &settings_;
    edit_.start(value, min, max, step, display_offset, off, on);
    edit_.print(lcd_buf[1] + 13);
    set_dirty(false);
}

inline void
controller_t::commit_edit()
{
    uint8_t *v = ((uint8_t *) &settings_) + edit_settings_offset_;
    *v = edit_.value();
    storage::write(settings_, *v);
}

inline void
controller_t::set_edit_loop(uint8_t edit_loop) {
    edit_loop_ = edit_loop;

    switch (mode_) {
    case MODE_SETTINGS_CTRL_IN:
        start_edit(settings_.midi_loop_ctrl_in[edit_loop_ - 1], 0, MAX_CTRL, 1, 0, MAX_CTRL);
        break;
    case MODE_SETTINGS_CTRL_OUT:
        start_edit(settings_.midi_loop_ctrl_out[edit_loop_ - 1], 0, MAX_CTRL, 1, 0,MAX_CTRL);
        break;
    }

    print_number(lcd_buf[1], 1, edit_loop_);

    out_.loop_led(1 << (edit_loop_ - 1));
}

inline void
controller_t::set_mode(uint8_t m) {
    debug(1, "set_mode: ", mode_, " -> ", m);

    if (m == mode_) return;

    if (mode_ == MODE_SETTINGS_FACTORY_RESET) {
        store_blink_timer_.stop();
    }

    if (mode_ == MODE_MIDI_IN_MONITOR ||
        mode_ == MODE_MIDI_OUT_MONITOR) {
        midi_mon_[mode_ - MODE_MIDI_IN_MONITOR].hide(t);
    }

    if (mode_ == MODE_MIDI_DUMP_RECV) {
        store_blink_timer_.stop();
        midi_mon_[0].clear();
        if (!midi_mon_[0].active()) {
            midi_mon_[0].toggle_pause(t);
        }
    }

    mode_ = m;

    if (show_master_) {
        show_master(false);
    }

    set_dirty(false);

    if (mode_ >= MODE_SETTINGS_FIRST &&
        mode_ <= MODE_SETTINGS_LAST) {
        memcpy(lcd_buf[0], " Settings       ", 16);
    }

    switch (mode_) {
    case MODE_INITIAL:
        break;

    case MODE_NORMAL:
        set_dirty(program_ != program_master_);

        print_number(lcd_buf[0], 3, prog_id_ + settings_.prog_start);
        lcd_buf[0][3] = ' ';
        memcpy(lcd_buf[0] + 4, program_.title, 12);
        lcd_buf[1][15] = ' ';
        break;

    case MODE_SETTINGS_CHANNEL:
        start_edit(settings_.midi_channel, 0, MAX_CHANNEL - 1, 1, settings_.chan_start);
        memcpy(lcd_buf[1], "MIDI Channel ", 13);
        break;

    case MODE_SETTINGS_CTRL_IN:
        set_edit_loop(1);
        memcpy(lcd_buf[1] + 1, " Ctrl IN    ", 12);
        break;

    case MODE_SETTINGS_CTRL_OUT:
        set_edit_loop(1);
        memcpy(lcd_buf[1] + 1, " Ctrl OUT   ", 12);
        break;

    case MODE_SETTINGS_PROG_OUT:
        start_edit(settings_.midi_out_prog, 0, 1, 1, 0, 0, 1);
        memcpy(lcd_buf[1], "Prog Chg OUT ", 13);
        break;

    case MODE_SETTINGS_MIDI_FWD:
        start_edit(settings_.midi_fwd, 0, 1, 1, 1, 0, 1);
        memcpy(lcd_buf[1], "MIDI Forward ", 13);
        break;

    case MODE_SETTINGS_MUTE_DELAY:
        start_edit(settings_.mute_delay_ms, 10, 900, 10);
        memcpy(lcd_buf[0] + 14, "ms", 2);
        memcpy(lcd_buf[1], " Mute delay  ", 13);
        break;

    case MODE_SETTINGS_HIDE_CURSOR:
        start_edit(settings_.hide_cursor_delay_s, 1, 60);
        lcd_buf[0][15] = 's';
        memcpy(lcd_buf[1], " Hide cursor ", 13);
        break;

    case MODE_SETTINGS_PROG_START:
        start_edit(settings_.prog_start, 0, 1);
        memcpy(lcd_buf[1], "Prog Start At ", 14);
        break;

    case MODE_SETTINGS_CHANNEL_START:
        start_edit(settings_.chan_start, 0, 1);
        memcpy(lcd_buf[1], "Chan Start At ", 14);
        break;

    case MODE_SETTINGS_FACTORY_RESET:
        memcpy(lcd_buf[1], " Factory Reset  ", 16);
        set_dirty(true);
        store_blink_start(0, 150);
        break;

#if defined(DEBUG)
    case MODE_SETTINGS_USB_DEBUG:
        start_edit(settings_.usb_debug, 0, 9);
        memcpy(lcd_buf[1], "USB Debug    ", 13);
        break;
#endif

    case MODE_PROGRAM_SWAP:
    case MODE_PROGRAM_MOVE:
        start_edit(prog_id_, 0, MAX_PROGRAMS - 1, 1, settings_.prog_start);

        print_number(lcd_buf[0], 3, prog_id_ + settings_.prog_start);
        lcd_buf[0][3] = ' ';
        memcpy(lcd_buf[0] + 4, program_.title, 12);

        print_number(lcd_buf[1] + 3, 3, prog_id_ + settings_.prog_start);
        lcd_buf[1][6] = ' ';
        memcpy(lcd_buf[1] + 7, program_.title, 9);

        memcpy(lcd_buf[1], mode_ == MODE_PROGRAM_SWAP ? "<> " : ">> ", 3);
        break;

    case MODE_MIDI_IN_MONITOR:
        midi_mon_[0].show(t, settings_.prog_start, settings_.chan_start);
        memcpy(lcd_buf[0], "MIDI IN Monitor ", 16);
        memcpy(lcd_buf[1], "0               ", 16);
        set_dirty(midi_mon_[0].active());  // turn 'STORE' led 'ON', capture enabled

        out_.loop_led(1 << midi_mon_[0].mode());
        break;

    case MODE_MIDI_OUT_MONITOR:
        midi_mon_[1].show(t, settings_.prog_start, settings_.chan_start);
        memcpy(lcd_buf[0], "MIDI OUT Monitor", 16);
        memcpy(lcd_buf[1], "0               ", 16);
        set_dirty(midi_mon_[1].active());  // turn 'STORE' led 'ON', capture enabled

        out_.loop_led(1 << midi_mon_[1].mode());
        break;

    case MODE_MIDI_DUMP_SEND:
        memcpy(lcd_buf[0], "MIDI Dump Send  ", 16);
        memcpy(lcd_buf[1], "  press Store   ", 16);
        break;

    case MODE_MIDI_DUMP_RECV:
        memcpy(lcd_buf[0], "MIDI Dump Recv  ", 16);
        memcpy(lcd_buf[1], "  waiting...    ", 16);

        dump_read_started_ = false;

        store_blink_start(0, 300);
        break;

    case MODE_ABOUT:
        memcpy(lcd_buf[0], "About PatchMate ", 16);
        version_.reset();
        version_.print_line();
        break;

    case MODE_UPTIME:
        memcpy(lcd_buf[0], " Uptime         ", 16);
        memcpy(lcd_buf[1] + 12, " sec", 4);
        uptime_ = t / 1000;
        print_number(lcd_buf[1], 12, uptime_);
        break;
    }

    // adjust loop led for all modes except MIDI mon and control settings
    if (led4loop()) {
        out_.loop_led(program_.loop);
    }

    hide_hint();
    hide_cursor();

    lcd_update();
/*
    if (mode_ == MODE_MIDI_IN_MONITOR ||
        mode_ == MODE_MIDI_OUT_MONITOR) {
        lcd.setCursor(2, 1);
        lcd.cursor();
    }
*/
}

inline void
controller_t::show_cursor()
{
    lcd.setCursor(cursor_pos_, 0);
    lcd.cursor();
    hide_cursor_timer_.schedule(t + settings_.hide_cursor_delay_s * 1000);
}

inline void
controller_t::hide_cursor()
{
    lcd.noCursor();
    hide_cursor_timer_.cancel();
}

inline void
controller_t::show_title_hint()
{
    char c = lcd_buf[0][cursor_pos_];

    for (uint8_t i = cursor_pos_; i < LCD_COLUMNS; ++i, c = rotate_char(c, 1)) {
        lcd_buf[1][i] = c;
    }

    c = rotate_char(lcd_buf[0][cursor_pos_], -1);

    for (uint8_t i = cursor_pos_; i > 0; --i, c = rotate_char(c, -1)) {
        lcd_buf[1][i - 1] = c;
    }

    c = lcd_buf[0][cursor_pos_];

    lcd_update(0, 1, LCD_COLUMNS);
}

inline void
controller_t::hide_hint()
{
    if (mode_ == MODE_NORMAL) {
        memset(lcd_buf[1], ' ', LCD_COLUMNS);
    }
}

inline void
controller_t::set_dirty(bool dirty)
{
    if (dirty == dirty_) return;

    debug(3, "dirty = ", dirty);

    dirty_ = dirty;
}

inline void
controller_t::show_master(bool master)
{
    show_master_ = master;

    if (mode_ != MODE_NORMAL) {
        return;
    }

    if (master) {
        memcpy(lcd_buf[0] + 4, program_master_.title, 12);
        out_.loop_led(program_master_.loop);
        hide_cursor();
        memcpy(lcd_buf[1], "    compare     ", LCD_COLUMNS);
    } else {
        memcpy(lcd_buf[0] + 4, program_.title, 12);
        out_.loop_led(program_.loop);
        memset(lcd_buf[1], ' ', LCD_COLUMNS);
    }

    lcd_update(0, 0, LCD_COLUMNS);
    lcd_update(0, 1, LCD_COLUMNS);
}

inline void
controller_t::midi_dump_send()
{
    dump_cmd_ << CMD_SYS_EX;

    dump_csum_ = DUMP_CSUM_START;

    midi_dump(eemagic, sizeof(eemagic));

    midi_dump_start_subject(DUMP_SETTINGS, sizeof(settings_));
    midi_dump(&settings_, sizeof(settings_));
    midi_dump_end_subject();

    program_t p;
    for (uint8_t i = 0; i < MAX_PROGRAMS; i++) {
        storage::read(i, p);
        midi_dump_start_subject(DUMP_PROGRAM, 1 + 2 + p.title_size());
        midi_dump(i);
        uint16_t l = p.loop;
        midi_dump(&l, sizeof(l));
        midi_dump(p.title, p.title_size());
        midi_dump_end_subject();
    }

    dump_cmd_ << CMD_SYS_EX_END;
    midi_dump_flush();
}

inline void
controller_t::midi_dump_start_subject(uint8_t subj, uint8_t size)
{
    size += ((size % 4) ? 4 - (size % 4) : 0);
    dump_subject_ = subj | (size / 4);
    dump_csum_ = DUMP_CSUM_START;

    midi_dump(dump_subject_);

    dump_pos_ = 0;
}

inline void
controller_t::midi_dump_end_subject()
{
    while (dump_pos_ < (dump_subject_ & DUMP_SIZE) * 4) { midi_dump(0x20); }
    midi_dump(dump_csum_);
}

inline void
controller_t::midi_dump(const void *d, uint8_t size)
{
    uint8_t *b = (uint8_t *) d;

    for (uint8_t i = 0; i < size; ++i) {
        midi_dump(b[i]);
    }
}

inline void
controller_t::midi_dump(uint8_t b)
{
    dump_csum_ += b;
    ++dump_pos_;

    debug(5, "mds:", dump_pos_, ",", b, ",", dump_csum_);

    if (b < 126) {
        dump_cmd_ << b;
        if (dump_cmd_.ready()) midi_dump_flush();
        return;
    }

    dump_cmd_ << (b > 127 ? 127 : 126);
    if (dump_cmd_.ready()) midi_dump_flush();

    dump_cmd_ << (b & 127);
    if (dump_cmd_.ready()) midi_dump_flush();
}

inline void
controller_t::midi_dump_flush()
{
    out_.usb_midi_write(dump_cmd_, 0);
    midi_out_cmd(dump_cmd_);

    dump_cmd_.reset();
}

inline void
controller_t::midi_dump_read(uint8_t b)
{
    if (is_midi_cmd(b)) {
        if (dump_subject_ != DUMP_NONE) {
            midi_dump_read_stop("bad format", 10, b);
        } else {
            midi_dump_read_stop("OK", 2, b);

            set_mode(MODE_NORMAL);
            set_program(prog_id_, true);
            store_blink_start();
        }
        return;
    }

    if (dump_8th_bit_) {
        b = (dump_8th_bit_ & 0x80) | b;
        dump_8th_bit_ = 0;
    } else {
        if (b == 127 || b == 126) {
            dump_8th_bit_ = ((b == 127) ? 0x80 : 0x00) | 0x01;
            return;
        }
    }

    dump_csum_ += b;

    debug(5, "mdr:", dump_pos_, ",", b, ",", dump_csum_);

    switch (dump_subject_ & DUMP_SUBJECT) {
    case DUMP_MAGIC:
        if (b != eemagic[dump_pos_]) {
            midi_dump_read_stop("bad magic", 10, b);
            return;
        }
        ++dump_pos_;
        if (dump_pos_ == sizeof(eemagic)) {
            dump_subject_ = DUMP_NONE;
            dump_csum_ = DUMP_CSUM_START;
        }
        break;

    case DUMP_NONE:
        if ((b & DUMP_SUBJECT) != DUMP_SETTINGS && (b & DUMP_SUBJECT) != DUMP_PROGRAM) {
            midi_dump_read_stop("bad subject", 11, b);
            return;
        }

        dump_subject_ = b;
        dump_pos_ = 0;
        break;

    case DUMP_SETTINGS:
    case DUMP_PROGRAM:
        if (dump_pos_ == 4 * (dump_subject_ & DUMP_SIZE)) {
            if (dump_csum_ != (uint8_t) (b + b)) {
                midi_dump_read_stop("bad checksum", 12, b);
                return;
            }

            if ((dump_subject_ & DUMP_SUBJECT) == DUMP_SETTINGS) {
                memcpy(&settings_, dump_buf_, sizeof(settings_));
                storage::write(settings_);
            } else {
                if (dump_buf_[0] < MAX_PROGRAMS) {
                    program_t p;
                    uint16_t l;
                    memcpy(&l, dump_buf_ + 1, 2);
                    p.loop = l;
                    uint8_t tsize = dump_pos_ - 3;
                    if (tsize > sizeof(p.title)) {
                        tsize = sizeof(p.title);
                    }
                    memcpy(p.title, dump_buf_ + 3, dump_pos_ - 3);
                    storage::write(dump_buf_[0], p);
                } else {
                    debug(3, "mdr ignore prog ", dump_buf_[0]);
                }
            }

            dump_subject_ = DUMP_NONE;
            dump_csum_ = DUMP_CSUM_START;
        } else {
            dump_buf_[dump_pos_++] = b;
        }
        break;
    }
}

inline void
controller_t::midi_dump_read_stop(const char *reason, uint8_t size, uint8_t b)
{
    debug(3, "mdr stop ", reason, ":", dump_pos_, ",", b, ",", dump_csum_);

    lcd_buf[1][0] = ' ';
    lcd_buf[1][1] = ' ';
    memcpy(lcd_buf[1] + 2, reason, size);
    memset(lcd_buf[1] + 2 + size, ' ', LCD_COLUMNS - 2 - size);

    lcd_update(0, 1, LCD_COLUMNS);

    dump_read_started_ = false;
}

inline void
controller_t::button_handler::on_down(bool down, unsigned long)
{
    controller_.on_down(id_, down);
}

inline void
controller_t::button_handler::on_hold(unsigned long)
{
    controller_.on_hold(id_);
}

inline void
controller_t::encoder_handler::on_rotate(short dir, unsigned long)
{
    controller_.on_rotate(dir);
}

inline void
controller_t::on_store_blink_timer()
{
    store_blink_timer_.led_on_ = !store_blink_timer_.led_on_;

    out_.store_led(store_blink_timer_.led_on_);

    if (!store_blink_timer_.led_on_) {
        if (store_blink_timer_.rest_ == 0) {
            return;
        }

        if (store_blink_timer_.rest_ != 0xFF) { store_blink_timer_.rest_--; }
    }

    store_blink_timer_.schedule(t + store_blink_timer_.period_);
}

inline void
controller_t::store_blink_start(uint8_t count, uint16_t period)
{
    store_blink_timer_.start(count, period);

    on_store_blink_timer();
}

inline void
controller_t::store_blink_timer_callback::start(uint8_t count, uint16_t period)
{
    led_on_ = false;
    rest_ = count - 1;
    period_ = period;
}

inline void
controller_t::store_blink_timer_callback::stop()
{
    rest_ = 0;
}

}
