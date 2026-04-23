#include "particle.hpp"
#include <cmath>
#include <algorithm>

static const float GRAVITY = 0.07f;
static const float DRAG    = 0.97f;

void Particle::update() {
    if (type == PType::STAR) {
        // Stars twinkle: sawtooth life oscillation
        life -= decay;
        if (life < 0.15f) life = 1.0f;
        return;
    }

    if (type == PType::GLITTER) {
        vel.y += GRAVITY * 0.4f;
        vel.x *= 0.99f;
        vel.y *= 0.99f;
    } else {
        vel.y += GRAVITY;
        vel.x *= DRAG;
        vel.y *= DRAG;
    }

    pos  += vel;
    life -= decay;
}

const char* Particle::glyph() const {
    switch (type) {
        case PType::ROCKET:
            return (life > 0.5f) ? "|" : "!";

        case PType::TRAIL:
            if (life > 0.65f) return "\xe2\x96\x93";  // ▓
            if (life > 0.35f) return "\xe2\x96\x92";  // ▒
            return "\xe2\x96\x91";                     // ░

        case PType::SPARK:
            if (life > 0.80f) return "\xe2\x96\x88";  // █
            if (life > 0.60f) return "\xe2\x96\x93";  // ▓
            if (life > 0.40f) return "*";
            if (life > 0.20f) return "+";
            if (life > 0.08f) return ".";
            return " ";

        case PType::GLITTER:
            if (life > 0.70f) return "\xe2\x97\x86";  // ◆
            if (life > 0.40f) return "+";
            return ".";

        case PType::STAR:
            if (life > 0.75f) return "*";
            if (life > 0.45f) return "+";
            if (life > 0.20f) return "\xc2\xb7";      // ·
            return ".";
    }
    return " ";
}

Color Particle::tintedColor() const {
    if (type == PType::STAR) {
        return color.dim(life * 0.85f + 0.15f);
    }
    float bright = std::min(1.0f, life * 1.3f);
    // Near death: shift toward orange/red
    if (type == PType::SPARK && life < 0.25f) {
        Color ember{200, 80, 10};
        return color.dim(bright).mix(ember, (0.25f - life) / 0.25f);
    }
    return color.dim(bright);
}
