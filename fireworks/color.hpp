#pragma once
#include <cstdint>
#include <string>
#include <cmath>
#include <algorithm>

struct Color {
    uint8_t r = 0, g = 0, b = 0;

    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    static Color fromHSV(float h, float s, float v) {
        h -= std::floor(h);
        int   hi = (int)(h * 6.0f);
        float f  = h * 6.0f - hi;
        float p  = v*(1-s), q = v*(1-f*s), t = v*(1-(1-f)*s);
        float cr, cg, cb;
        switch (hi % 6) {
            case 0: cr=v; cg=t; cb=p; break;
            case 1: cr=q; cg=v; cb=p; break;
            case 2: cr=p; cg=v; cb=t; break;
            case 3: cr=p; cg=q; cb=v; break;
            case 4: cr=t; cg=p; cb=v; break;
            default: cr=v; cg=p; cb=q; break;
        }
        return {(uint8_t)(cr*255), (uint8_t)(cg*255), (uint8_t)(cb*255)};
    }

    Color dim(float f) const {
        f = std::max(0.0f, std::min(1.0f, f));
        return {(uint8_t)(r*f), (uint8_t)(g*f), (uint8_t)(b*f)};
    }

    Color mix(const Color& o, float t) const {
        return {(uint8_t)(r+(o.r-r)*t), (uint8_t)(g+(o.g-g)*t), (uint8_t)(b+(o.b-b)*t)};
    }

    bool operator==(const Color& o) const { return r==o.r && g==o.g && b==o.b; }
    bool operator!=(const Color& o) const { return !(*this == o); }

    std::string ansi() const {
        return "\033[38;2;" + std::to_string((int)r) + ";"
                            + std::to_string((int)g) + ";"
                            + std::to_string((int)b) + "m";
    }
};
