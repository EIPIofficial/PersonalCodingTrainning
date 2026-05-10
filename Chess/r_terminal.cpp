// This will happen if you exit the game irregularly

#include <iostream>
#include <termios.h>
#include <unistd.h>

int main() {
    termios state{};
    if (tcgetattr(STDIN_FILENO, &state) == 0) {
        state.c_lflag |= ICANON | ECHO | ISIG | IEXTEN;
        state.c_iflag |= ICRNL | IXON;
        state.c_oflag |= OPOST;
        state.c_cc[VMIN] = 1;
        state.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &state);
    }

    std::cout
        << "\x1b[0m"
        << "\x1b[?25h"
        << "\x1b[?1000l"
        << "\x1b[?1002l"
        << "\x1b[?1003l"
        << "\x1b[?1005l"
        << "\x1b[?1006l"
        << "\x1b[?1015l"
        << "\x1b[?1049l"
        << std::flush;

    return 0;
}
