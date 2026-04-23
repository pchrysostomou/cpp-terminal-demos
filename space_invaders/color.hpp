#pragma once
#include <cstdint>
#include <string>
#include <algorithm>

struct Color {
    uint8_t r = 0, g = 0, b = 0;
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    bool operator==(const Color& o) const { return r==o.r && g==o.g && b==o.b; }
    bool operator!=(const Color& o) const { return !(*this == o); }

    std::string fg() const {
        return "\033[38;2;" + std::to_string((int)r) + ";"
                            + std::to_string((int)g) + ";"
                            + std::to_string((int)b) + "m";
    }
    std::string bg() const {
        return "\033[48;2;" + std::to_string((int)r) + ";"
                            + std::to_string((int)g) + ";"
                            + std::to_string((int)b) + "m";
    }
};
