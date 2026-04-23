#include "input.hpp"
#include <termios.h>
#include <unistd.h>

static struct termios g_old;

void inputInit() {
    tcgetattr(STDIN_FILENO, &g_old);
    struct termios raw = g_old;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;   // return immediately even with 0 bytes
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void inputCleanup() {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_old);
}

int inputRead() {
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) <= 0) return KEY_NONE;

    if (c == 27) {
        char s[2] = {0, 0};
        if (read(STDIN_FILENO, &s[0], 1) == 1 && s[0] == '[') {
            if (read(STDIN_FILENO, &s[1], 1) == 1) {
                switch (s[1]) {
                    case 'A': return KEY_UP;
                    case 'B': return KEY_DOWN;
                    case 'C': return KEY_RIGHT;
                    case 'D': return KEY_LEFT;
                }
            }
        }
        return KEY_ESC;
    }
    return (unsigned char)c;
}
