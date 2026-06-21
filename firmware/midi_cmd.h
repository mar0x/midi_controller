#pragma once

#include <stdint.h>

namespace midi_controller {

enum {
    CMD_BIT           = 0x80U,

    CMD_NOTE_OFF      = 0x80U,
    CMD_NOTE_ON       = 0x90U,
    CMD_KEY_PRESSURE  = 0xA0U,
    CMD_CTRL_CHANGE   = 0xB0U,
    CMD_PROG_CHANGE   = 0xC0U,
    CMD_CHAN_PRESSURE = 0xD0U,
    CMD_PITCH_CHANGE  = 0xE0U,

    CMD_SYS           = 0xF0U,

    CMD_SYS_EX        = 0xF0U,
    CMD_SYS_EX_END    = 0xF7U,

    CMD_SYS_MTC       = 0xF1U,
    CMD_SYS_SONG_PP   = 0xF2U,
    CMD_SYS_SONG_SEL  = 0xF3U,
    CMD_SYS_TUNE_REQ  = 0xF6U,

    CMD_SYS_RT        = 0xF8U,

    CMD_SYS_CLOCK     = 0xF8U,
    CMD_SYS_TICK      = 0xF9U,
    CMD_SYS_START     = 0xFAU,
    CMD_SYS_CONT      = 0xFBU,
    CMD_SYS_STOP      = 0xFCU,
    CMD_SYS_UNDEF     = 0xFDU,
    CMD_SYS_ACTIVE_S  = 0xFEU,
    CMD_SYS_RESET     = 0xFFU,
};

enum {
    CTRL_ALL_SOUND_OFF  = 120,
    CTRL_RESET_ALL      = 121,
    CTRL_LOCAL_CONTROL  = 122,
};

inline bool is_midi_rt(uint8_t b) {
    return (b & CMD_SYS_RT) == CMD_SYS_RT;
}

inline bool is_midi_cmd(uint8_t b) {
    return (b & CMD_BIT) == CMD_BIT;
}

inline bool is_midi_sys(uint8_t b) {
    return (b & CMD_SYS) == CMD_SYS;
}

struct midi_cmd_t {
    midi_cmd_t() = default;
    midi_cmd_t(uint8_t ch, uint8_t cmd);
    midi_cmd_t(uint8_t ch, uint8_t cmd, uint8_t val);
    midi_cmd_t(uint8_t ch, uint8_t cmd, uint8_t ctl, uint8_t val);

    static uint8_t command(uint8_t b);
    static uint8_t size(uint8_t b);

    bool sys_ex() const { return sys_ex_ || (size_ > 0 && command() == CMD_SYS_EX); }
    void sys_ex(bool v) { sys_ex_ = v; }

    uint8_t command() const { return command(cmd_[0]); }
    uint8_t channel() const { return cmd_[0] & 0x0FU; }
    uint8_t controller() const { return cmd_[1]; }
    uint8_t value() const { return cmd_[2]; }
    uint8_t program() const { return cmd_[1]; }
    uint8_t key() const { return cmd_[1]; }
    uint16_t pitch() const { return cmd_[1] | (cmd_[2] << 7); }

    uint8_t size() const { return size_; }

    bool ready() const;

    operator const uint8_t*() const { return cmd_; }
    operator uint8_t*() { return cmd_; }

    bool operator==(const midi_cmd_t& cmd) const;

    midi_cmd_t &operator<<(uint8_t b);

    void reset();
    void read(uint8_t b);

private:
    enum {
        MAX_SIZE = 3
    };

    uint8_t cmd_[MAX_SIZE];
    uint8_t size_ = 0;
    bool sys_ex_ = false;
};

}
