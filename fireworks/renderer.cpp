#include "renderer.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Renderer::Renderer(int w, int h) : W_(w), H_(h) {
    buf_.assign(h, std::vector<Cell>(w));
}

void Renderer::clearBuf() {
    static const Color BG{3, 3, 14};
    for (auto& row : buf_)
        for (auto& c : row) { c.glyph = " "; c.color = BG; c.zorder = 0; }
}

void Renderer::plot(const Particle& p, int z) {
    int x = (int)std::round(p.pos.x);
    int y = (int)std::round(p.pos.y);
    if (x < 0 || x >= W_ || y < 0 || y >= H_) return;

    Cell& cell = buf_[y][x];
    if (z >= cell.zorder) {
        cell.glyph  = p.glyph();
        cell.color  = p.tintedColor();
        cell.zorder = z;
    }
}

void Renderer::render(const Scene& sc, double fps) {
    clearBuf();

    // Stars (z=1)
    for (auto& s : sc.stars)
        if (s.alive()) plot(s, 1);

    // Firework particles
    for (auto& fw : sc.fireworks) {
        for (auto& p : fw.particles) {
            if (!p.alive()) continue;
            int z = (p.type == PType::ROCKET)  ? 10 :
                    (p.type == PType::TRAIL)    ?  5 :
                    (p.type == PType::GLITTER)  ?  9 : 8;
            plot(p, z);
        }
    }

    flush(sc, fps);
}

void Renderer::flush(const Scene& sc, double fps) {
    std::string out;
    out.reserve(W_ * H_ * 22);
    out += "\033[H";  // cursor home (redraw in-place)

    Color lastColor{0, 0, 0};
    bool  newLine = true;

    for (int y = 0; y < H_; ++y) {
        for (int x = 0; x < W_; ++x) {
            const Cell& cell = buf_[y][x];
            if (newLine || cell.color != lastColor) {
                out += cell.color.ansi();
                lastColor = cell.color;
                newLine   = false;
            }
            out += cell.glyph;
        }
        out += "\033[0m\n";
        newLine = true;
    }

    // ── Status bar ──────────────────────────────────────────────
    int totalParticles = 0;
    for (auto& fw : sc.fireworks)
        totalParticles += (int)fw.particles.size();

    out += "\033[0m\033[1;33m";
    out += " \xe2\x9c\xa6 FIREWORKS  ";                  // ✦
    out += "\033[0m\033[33m";
    out += " Frame:"   + std::to_string(sc.frame());
    out += "  FPS:"    + std::to_string((int)fps);
    out += "  Active:" + std::to_string(sc.fireworks.size());
    out += "  Particles:" + std::to_string(totalParticles);
    out += (sc.frame() >= 600 ? "  \033[1;31m\xe2\x97\x86 FINALE!\033[0m" : "");
    out += "  \033[90mCtrl+C = exit\033[0m          ";

    std::cout << out << std::flush;
}
