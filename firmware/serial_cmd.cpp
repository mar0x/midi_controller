#include "serial_cmd.h"
#include "debug.h"

#include <avr/pgmspace.h>

namespace midi_controller {

void
serial_cmd_t::read(uint8_t b) {
    if (b == '\n' || b == '\r') {
        buf_[size_] = 0;

        parse();
        ready_ = true;

        return;
    }

    if (size_ >= MAX_SIZE) {
        return;
    }

    buf_[size_++] = b;
}

void
serial_cmd_t::parse() {
    arg_size_ = 0;
    arg *a = &arg_[0];
    char in_quote = 0;

    a->start = 0;

    for (uint8_t i = 0; i < size_; i++) {
        char b = buf_[i];

        if (b == '"' && i == a->start) {
            a->start = i + 1;
            in_quote = b;
            continue;
        }

        if (in_quote && in_quote == b) {
            a->end = i;

            debug(7, "arg #", arg_size_, ", start ", a->start, ", end ", a->end);
            //log_window::println("arg #", arg_size_, ", start ", a->start, ", end ", a->end);

            arg_size_++;
            a++;

            a->start = i + 1;
            in_quote = 0;

            if (arg_size_ >= MAX_ARGS) {
                return;
            }
        }

        if (in_quote == 0 && (b == ' ' || b == '\t')) {
            if (i == a->start) {
                a->start = i + 1;
                continue;
            }

            a->end = i;

            //log_window::println("arg #", arg_size_, ", start ", a->start, ", end ", a->end);

            arg_size_++;
            a++;

            a->start = i + 1;

            if (arg_size_ >= MAX_ARGS) {
                return;
            }
        }
    }

    if (a->start < size_) {

        a->end = size_;

        //log_window::println("arg #", arg_size_, ", start ", a->start, ", end ", a->end);

        arg_size_++;
    }

    if (arg_size_ > 0) {
        a = &arg_[0];
        char *b = &buf_[a->start];

        switch (a->end - a->start) {
        case 1:
            if (b[0] == 'V') { command_ = CMD_VERSION; }
            if (b[0] == 'E') { command_ = CMD_ECHO; }
            if (b[0] == 'B') { command_ = CMD_BTN_PRESS; }
            if (b[0] == 'D') { command_ = CMD_DISPLAY_DUMP; }
            if (b[0] == '?') { command_ = CMD_HELP; }
            break;

        case 2:
            if (b[0] == 'P' && b[1] == 'C') { command_ = CMD_PROG_CHANGE; }
            if (b[0] == 'C' && b[1] == 'C') { command_ = CMD_CTRL_CHANGE; }
            if (b[0] == 'L' && b[1] == 'P') { command_ = CMD_LOOP; }
            if (b[0] == 'N' && b[1] == 'M') { command_ = CMD_NAME; }
            if (b[0] == 'M' && b[1] == 'D') { command_ = CMD_MODE; }
            if (b[0] == 'S' && b[1] == 'T') { command_ = CMD_STORE; }
            if (b[0] == 'R' && b[1] == 'S') { command_ = CMD_RESTORE; }
            if (b[0] == 'P' && b[1] == 'R') { command_ = CMD_PROGRAM; }
            if (b[0] == 'M' && b[1] == 'C') { command_ = CMD_MIDI_CHANNEL; }
            if (b[0] == 'M' && b[1] == 'O') { command_ = CMD_MIDI_PROG_OUT; }
            if (b[0] == 'M' && b[1] == 'F') { command_ = CMD_MIDI_FORWARD; }
            if (b[0] == 'D' && b[1] == 'L') { command_ = CMD_DEBUG_LEVEL; }
            if (b[0] == 'M' && b[1] == 'L') { command_ = CMD_MUTE_DELAY; }
            if (b[0] == 'H' && b[1] == 'C') { command_ = CMD_HIDE_CURSOR_DELAY; }
            if (b[0] == 'F' && b[1] == 'R') { command_ = CMD_FACTORY_RESET; }
            if (b[0] == 'S' && b[1] == 'N') { command_ = CMD_SERIAL_NUMBER; }
            if (b[0] == 'H' && b[1] == 'W') { command_ = CMD_HARDWARE; }
            break;

        case 3:
            if (b[0] == 'M' && b[1] == 'L' && b[2] == 'I') { command_ = CMD_MIDI_LOOP_IN_CTRL; }
            if (b[0] == 'M' && b[1] == 'L' && b[2] == 'O') { command_ = CMD_MIDI_LOOP_OUT_CTRL; }
            if (b[0] == 'M' && b[1] == 'M' && b[2] == 'I') { command_ = CMD_MIDI_MON_IN; }
            if (b[0] == 'M' && b[1] == 'M' && b[2] == 'O') { command_ = CMD_MIDI_MON_OUT; }
            if (b[0] == 'M' && b[1] == 'D' && b[2] == 'S') { command_ = CMD_MIDI_DUMP_SEND; }
            if (b[0] == 'M' && b[1] == 'D' && b[2] == 'R') { command_ = CMD_MIDI_DUMP_RECV; }
            break;
        }
    }
}


static const char help_[] PROGMEM = R"HELP(
 MIDI Simulation:
PC [<P>] - program change
CC <C> [<V>] - controller change

 Current state change:
LP [<L> [<V>]] - loop change
NM [<NAME>] - name change
MD [<MODE>] - mode change
ST - store changes
RS - restore
MDS - MIDI Dump Send
MDR - MIDI Dump Receive

PR [<P> [<T> [<NAME>]]]

 Simulation:
B [L/R/U/D/S] - button press
D - display dump

 Settings:
MC [<C>] - MIDI channel
MLI <T> [<C>] - MIDI in loop controller
MLO <T> [<C>] - MIDI out loop controller
MO [1/0] - MIDI out program change
MF [1/0] - MIDI forwarding
DL [<L>] - debug level
ML [<s>] - mute delay
HC [<s>] - hint delay
E [1/0] - echo
V - show version

FR <V> - factory reset
)HELP";

}
