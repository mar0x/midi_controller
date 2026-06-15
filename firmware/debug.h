
#pragma once

#define DEBUG 1
#define MAX_DEBUG_LEVEL 0

/*
    1    set program
    1    set loop
    1    set mode

    2    rotate value
    2    toggle loop (on_down)

    3    set_dirty
    3    mdr stop

    4    modi_cmd read byte
    4    modi_cmd write byte
    4    on_down
    4    on_hold
    4    on_rotate (encoder)

    5    on_down (release key)
    5    mds (midi dump store byte)
    5    mdr (midi dump read byte)

    6    midi read
    6    usb write
    6    loop_state read prog
    6    loop_state write prog

    7    in:update value
    7    out: mute
    7    out: unmute
    7    out: commit sipo
    7    out: commit_led only
    7    program::setup start
    7    program::read prog
    7    program::write prog
 */


#if defined(DEBUG) && defined(HAVE_HWSERIAL1)

static uint8_t debug_level_ = 0;

template<typename T1> void debug__(const T1& a1) {
    if (Serial.dtr()) {
        Serial.println(a1);
    }
}

template<typename T1, typename ...Args> void debug__(const T1& a1, Args... args) {
    if (Serial.dtr()) {
        Serial.print(a1);
        debug__(args...);
    }
}

template<typename T1, typename ...Args> void debug(uint8_t l, const T1& a1, Args... args) {
    if (l <= MAX_DEBUG_LEVEL && l <= debug_level_) {
        debug__(a1, args...);
    }
}

#else

#define debug(...)

#endif
