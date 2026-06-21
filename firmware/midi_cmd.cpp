#include "midi_cmd.h"
#include <string.h>

namespace midi_controller {

midi_cmd_t::midi_cmd_t(uint8_t ch, uint8_t cmd) :
    size_(1)
{
    cmd_[0] = cmd | (ch & 0x0FU);
}

midi_cmd_t::midi_cmd_t(uint8_t ch, uint8_t cmd, uint8_t val) :
    size_(2)
{
    cmd_[0] = cmd | (ch & 0x0FU);
    cmd_[1] = val & 0x7FU;
}

midi_cmd_t::midi_cmd_t(uint8_t ch, uint8_t cmd, uint8_t ctl, uint8_t val) :
    size_(3)
{
    cmd_[0] = cmd | (ch & 0x0FU);
    cmd_[1] = ctl & 0x7FU;
    cmd_[2] = val & 0x7FU;
}

uint8_t midi_cmd_t::command(uint8_t b) {
    if (is_midi_cmd(b)) {
        uint8_t c = b & CMD_SYS;
        return c != CMD_SYS ? c : b;
    }

    return 0;
}

uint8_t midi_cmd_t::size(uint8_t b) {
    switch (command(b)) {
    case CMD_NOTE_OFF:
    case CMD_NOTE_ON:
    case CMD_KEY_PRESSURE:
    case CMD_CTRL_CHANGE:
    case CMD_PITCH_CHANGE:
    case CMD_SYS_SONG_PP:
        return 3;

    case CMD_PROG_CHANGE:
    case CMD_CHAN_PRESSURE:
    case CMD_SYS_MTC:
    case CMD_SYS_SONG_SEL:
    case CMD_SYS_EX:
        return 2;

    case CMD_SYS_EX_END:
    case CMD_SYS_CLOCK:
    case CMD_SYS_TICK:
    case CMD_SYS_START:
    case CMD_SYS_CONT:
    case CMD_SYS_STOP:
    case CMD_SYS_UNDEF:
    case CMD_SYS_ACTIVE_S:
    case CMD_SYS_RESET:
        return 1;
    }

    return 0;
}

bool midi_cmd_t::ready() const {
    if (size_ == 0) return false;
    if (size_ == MAX_SIZE) return true;

    if (is_midi_cmd(cmd_[0]) && cmd_[0] != CMD_SYS_EX) {
        return size_ == size(cmd_[0]);
    }

    return cmd_[size_ - 1] == CMD_SYS_EX_END;
}

bool midi_cmd_t::operator==(const midi_cmd_t& cmd) const {
    return size_ == cmd.size_ && memcmp(cmd_, cmd.cmd_, size_) == 0;
}

midi_cmd_t &midi_cmd_t::operator<<(uint8_t b) {
    read(b);
    return *this;
}

void midi_cmd_t::reset() {
    if (size_ == MAX_SIZE && command() == CMD_SYS_EX && cmd_[size_ - 1] != CMD_SYS_EX_END) {
        sys_ex_ = true;
    }

    if (size_ > 0 && cmd_[size_ - 1] == CMD_SYS_EX_END) {
        sys_ex_ = false;
    }

    size_ = 0;
}

void midi_cmd_t::read(uint8_t b) {
    if (is_midi_cmd(b) && (b != CMD_SYS_EX_END || size_ == 0)) {
        cmd_[0] = b;
        size_ = 1;
        sys_ex_ = false;

        return;
    }

    if ((size_ > 0 || sys_ex_) && size_ < MAX_SIZE) {
        cmd_[size_++] = b;
    }
}

}
