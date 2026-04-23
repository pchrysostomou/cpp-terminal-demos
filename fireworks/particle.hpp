#pragma once
#include "math.hpp"
#include "color.hpp"

enum class PType { ROCKET, SPARK, TRAIL, STAR, GLITTER };

struct Particle {
    Vec2   pos, vel;
    float  life  = 0.0f;   // 1.0 = fresh  →  0.0 = dead
    float  decay = 0.02f;  // life decrease per frame
    Color  color;
    PType  type  = PType::SPARK;

    bool        alive()        const { return life > 0.0f; }
    void        update();
    const char* glyph()        const;
    Color       tintedColor()  const;
};
