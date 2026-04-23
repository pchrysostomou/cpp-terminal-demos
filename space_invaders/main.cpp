// ╔══════════════════════════════════════════════════════════╗
// ║           SPACE INVADERS — Terminal C++ Edition          ║
// ║  Arrow Keys / WASD = move  |  SPACE / W = shoot         ║
// ║  R = restart  |  Q = quit                               ║
// ╠══════════════════════════════════════════════════════════╣
// ║  Build:  make                                           ║
// ║  Run:    ./invaders                                     ║
// ╚══════════════════════════════════════════════════════════╝

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <ctime>

#ifdef __unix__
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

#include "game.hpp"
#include "renderer.hpp"
#include "input.hpp"

static bool g_running = true;

static void onExit(int) {
    g_running = false;
}

static std::pair<int,int> termSize() {
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 20)
        return {ws.ws_col, ws.ws_row - 1};
#endif
    return {80, 24};
}

int main() {
    std::srand((unsigned)std::time(nullptr));
    std::signal(SIGINT, onExit);

    inputInit();

    auto [W, H] = termSize();
    std::cout << "\033[2J\033[?25l";   // clear + hide cursor

    Game     game(W, H);
    Renderer renderer(W, H);

    using Clock = std::chrono::steady_clock;
    const int FPS      = 20;
    const int FRAME_MS = 1000 / FPS;

    while (g_running) {
        auto t0 = Clock::now();

        int key = inputRead();
        if (key == KEY_Q || key == KEY_ESC) break;

        game.update(key);
        renderer.draw(game);

        auto   t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        int    sl = std::max(0, FRAME_MS - (int)ms);
        if (sl > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sl));
    }

    inputCleanup();
    std::cout << "\033[?25h\033[0m\n\n"
              << "  \033[32mThanks for playing! Final score: "
              << game.score() << "\033[0m\n\n";
    return 0;
}
