// ╔═══════════════════════════════════════════════════════════╗
// ║         C++ FIREWORKS SIMULATOR  —  Terminal Edition      ║
// ║  5 explosion patterns · Physics · Stars · Grand Finale    ║
// ╠═══════════════════════════════════════════════════════════╣
// ║  Build:   make                                            ║
// ║  Run:     ./fireworks                                     ║
// ║  Quit:    Ctrl+C                                          ║
// ╚═══════════════════════════════════════════════════════════╝

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

#include "scene.hpp"
#include "renderer.hpp"

// ─────────────────────────────────────────────
//  Signal handler — restore terminal on Ctrl+C
// ─────────────────────────────────────────────
static void onExit(int) {
    std::cout << "\033[?25h\033[0m\n\n"
              << " \033[1;33m\xe2\x9c\xa6\033[0m  "    // ✦
              << "\033[32mFireworks show complete!\033[0m\n\n";
    std::exit(0);
}

// ─────────────────────────────────────────────
//  Detect terminal dimensions
// ─────────────────────────────────────────────
static std::pair<int,int> termSize() {
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 10)
        return {ws.ws_col, ws.ws_row - 2};
#endif
    return {80, 22};
}

// ─────────────────────────────────────────────
//  Intro banner
// ─────────────────────────────────────────────
static void showIntro() {
    const char* lines[] = {
        "",
        "  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 FIREWORKS SIMULATOR \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88",
        "",
        "  5 explosion patterns:",
        "    \xe2\x97\x86 BURST          classic scatter",
        "    \xe2\x97\x86 WILLOW         drooping arcs",
        "    \xe2\x97\x86 RING           tight circle",
        "    \xe2\x97\x86 CHRYSANTHEMUM  double rings",
        "    \xe2\x97\x86 GLITTER        diamond sparks",
        "",
        "  At frame 600  \xe2\x86\x92  GRAND FINALE !",
        "",
    };

    const char* colors[] = {
        "\033[31m","\033[33m","\033[32m",
        "\033[36m","\033[34m","\033[35m"
    };

    for (int flash = 0; flash < 5; ++flash) {
        std::cout << "\033[2J\033[H" << colors[flash % 6];
        for (auto& l : lines) std::cout << l << "\n";
        std::cout << "\033[0m" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main() {
    std::srand((unsigned)std::time(nullptr));
    std::signal(SIGINT, onExit);

    showIntro();

    auto [W, H] = termSize();

    std::cout << "\033[2J\033[?25l";  // clear screen, hide cursor

    Scene    scene(W, H);
    Renderer renderer(W, H);

    using Clock = std::chrono::steady_clock;
    const int TARGET_FPS = 20;
    const int FRAME_MS   = 1000 / TARGET_FPS;

    double smoothFPS = TARGET_FPS;

    while (true) {
        auto t0 = Clock::now();

        scene.update();
        renderer.render(scene, smoothFPS);

        auto t1  = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        smoothFPS = smoothFPS * 0.85 + (1000.0 / std::max(1.0, ms)) * 0.15;

        int sleep = std::max(0, FRAME_MS - (int)ms);
        if (sleep > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}
