#pragma once

#include <stdint.h>
#include "profile.h"
#include "program.h"
#include "config.h"
#include "serial_cmd.h"

namespace midi_controller {

extern uint8_t cur_profile_id;
extern profile_t cur_profile;
extern uint8_t select_profile_id;

extern uint8_t cur_program_id;
extern uint8_t cur_program_seq;
extern program_t cur_program;

void show_profile(uint8_t profile_id);
void show_program(uint8_t p, uint8_t s = MAX_PROGRAM, bool show_banner = false);
void set_program(uint8_t p, uint8_t s = MAX_PROGRAM, bool show_banner = false);
void process_cmd(const serial_cmd_t &cmd, bool output_reply);

}
