#include "firework.hpp"
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Firework::Firework(float x, float groundY, float hue)
    : state_(FWState::LAUNCHING), hue_(hue)
{
    int r = randi(0, 4);
    pattern_ = static_cast<FWPattern>(r);

    Particle rocket;
    rocket.pos   = {x, groundY};
    rocket.vel   = {randf(-0.4f, 0.4f), randf(-1.3f, -1.9f)};
    rocket.life  = 1.0f;
    rocket.decay = 0.001f;
    rocket.color = Color{220, 230, 255};
    rocket.type  = PType::ROCKET;
    rocketIdx_   = 0;
    particles.reserve(200);   // pre-allocate: prevents reallocation + dangling refs
    particles.push_back(rocket);
}

// ─────────────────────────────────────────────
//  Per-frame update
// ─────────────────────────────────────────────
void Firework::update() {
    if (state_ == FWState::LAUNCHING) {
        // Snapshot position/velocity BEFORE push_back (which could reallocate)
        Vec2 rpos = particles[rocketIdx_].pos;
        Vec2 rvel = particles[rocketIdx_].vel;

        Particle trail;
        trail.pos   = rpos;
        trail.vel   = {rvel.x * 0.1f, randf(0.0f, 0.25f)};
        trail.life  = randf(0.6f, 0.9f);
        trail.decay = randf(0.055f, 0.085f);
        trail.color = Color::fromHSV(0.60f, 0.4f, 0.95f);
        trail.type  = PType::TRAIL;
        particles.push_back(trail);

        // Access rocket by index (safe: reserve(200) prevents reallocation)
        particles[rocketIdx_].update();

        if (particles[rocketIdx_].vel.y >= -0.04f || !particles[rocketIdx_].alive()) {
            explode();
            state_ = FWState::EXPLODED;
        }

    } else if (state_ == FWState::EXPLODED) {
        bool any = false;
        for (auto& p : particles) {
            if (p.alive()) { p.update(); any = true; }
        }
        if (!any) state_ = FWState::DONE;
    }
}

// ─────────────────────────────────────────────
//  Explosion dispatcher
// ─────────────────────────────────────────────
void Firework::explode() {
    particles[rocketIdx_].life = 0.0f;   // kill rocket
    switch (pattern_) {
        case FWPattern::BURST:         burst();        break;
        case FWPattern::WILLOW:        willow();       break;
        case FWPattern::RING:          ring();         break;
        case FWPattern::CHRYSANTHEMUM: chrysanthemum();break;
        case FWPattern::GLITTER:       glitter();      break;
    }
}

// ─────────────────────────────────────────────
//  Burst — classic random scatter
// ─────────────────────────────────────────────
void Firework::burst() {
    Vec2  center = particles[rocketIdx_].pos;
    int   n      = randi(55, 85);

    for (int i = 0; i < n; ++i) {
        float angle = randf(0, 2.0f*(float)M_PI);
        float speed = randf(0.2f, 1.1f);
        if (randf() < 0.15f) speed *= 2.0f;   // a few fast outliers

        Particle s;
        s.pos   = center;
        s.vel   = {std::cos(angle)*speed, std::sin(angle)*speed*0.5f};
        s.life  = randf(0.7f, 1.0f);
        s.decay = randf(0.007f, 0.018f);
        s.color = Color::fromHSV(hue_ + randf(-0.04f, 0.04f),
                                 randf(0.75f, 1.0f), randf(0.85f, 1.0f));
        s.type  = PType::SPARK;
        particles.push_back(s);
    }
}

// ─────────────────────────────────────────────
//  Willow — drooping weeping-willow arcs
// ─────────────────────────────────────────────
void Firework::willow() {
    Vec2 center = particles[rocketIdx_].pos;
    int  n      = randi(50, 70);

    for (int i = 0; i < n; ++i) {
        float angle = randf(0, 2.0f*(float)M_PI);
        float speed = randf(0.4f, 0.95f);

        Particle s;
        s.pos   = center;
        // Bias strongly upward so arcs droop back down beautifully
        s.vel   = {std::cos(angle)*speed, std::sin(angle)*speed*0.5f - 0.6f};
        s.life  = randf(0.8f, 1.0f);
        s.decay = randf(0.005f, 0.012f);   // willow sparks live longer
        s.color = Color::fromHSV(hue_, randf(0.5f, 0.85f), 1.0f);
        s.type  = PType::SPARK;
        particles.push_back(s);
    }
}

// ─────────────────────────────────────────────
//  Ring — tight circular shell
// ─────────────────────────────────────────────
void Firework::ring() {
    Vec2  center = particles[rocketIdx_].pos;
    int   n      = randi(48, 64);
    float speed  = randf(0.7f, 1.0f);

    for (int i = 0; i < n; ++i) {
        float angle = (float)i / n * 2.0f*(float)M_PI;
        float spd   = speed + randf(-0.05f, 0.05f);

        Particle s;
        s.pos   = center;
        s.vel   = {std::cos(angle)*spd, std::sin(angle)*spd*0.5f};
        s.life  = randf(0.85f, 1.0f);
        s.decay = randf(0.006f, 0.014f);
        s.color = Color::fromHSV(hue_ + (float)i/n*0.12f, 0.9f, 1.0f);
        s.type  = PType::SPARK;
        particles.push_back(s);
    }
}

// ─────────────────────────────────────────────
//  Chrysanthemum — two concentric rings
// ─────────────────────────────────────────────
void Firework::chrysanthemum() {
    Vec2 center = particles[rocketIdx_].pos;

    auto makeRing = [&](int n, float speed, float hueShift) {
        for (int i = 0; i < n; ++i) {
            float angle = (float)i / n * 2.0f*(float)M_PI;
            Particle s;
            s.pos   = center;
            s.vel   = {std::cos(angle)*speed, std::sin(angle)*speed*0.5f};
            s.life  = randf(0.8f, 1.0f);
            s.decay = randf(0.006f, 0.013f);
            s.color = Color::fromHSV(hue_ + hueShift, 0.85f, 1.0f);
            s.type  = PType::SPARK;
            particles.push_back(s);
        }
    };

    makeRing(40, 0.5f, 0.0f);           // inner ring
    makeRing(60, 1.1f, 0.08f);          // outer ring (slightly different hue)
}

// ─────────────────────────────────────────────
//  Glitter — slow-falling diamond sparks
// ─────────────────────────────────────────────
void Firework::glitter() {
    Vec2 center = particles[rocketIdx_].pos;
    int  n      = randi(60, 90);

    for (int i = 0; i < n; ++i) {
        float angle = randf(0, 2.0f*(float)M_PI);
        float speed = randf(0.1f, 0.8f);

        Particle s;
        s.pos   = center;
        s.vel   = {std::cos(angle)*speed, std::sin(angle)*speed*0.4f};
        s.life  = randf(0.7f, 1.0f);
        s.decay = randf(0.004f, 0.011f);   // glitter lives longest
        s.color = Color::fromHSV(hue_ + randf(-0.06f, 0.06f), 0.6f, 1.0f);
        s.type  = PType::GLITTER;
        particles.push_back(s);
    }
}
