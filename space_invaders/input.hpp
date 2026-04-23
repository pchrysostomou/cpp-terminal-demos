#pragma once

enum Key {
    KEY_NONE  =   0,
    KEY_LEFT  =   1,
    KEY_RIGHT =   2,
    KEY_UP    =   3,
    KEY_DOWN  =   4,
    KEY_SPACE =  32,
    KEY_ESC   =  27,
    KEY_Q     = 'q',
    KEY_R     = 'r',
    KEY_ENTER =  13
};

void inputInit();      // call once at startup
void inputCleanup();   // call on exit to restore terminal
int  inputRead();      // non-blocking; returns Key or raw char, 0 if nothing
