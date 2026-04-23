#pragma once
#include <vector>
#include "particle.hpp"

enum class FWState   { LAUNCHING, EXPLODED, DONE };
enum class FWPattern { BURST, WILLOW, RING, CHRYSANTHEMUM, GLITTER };

class Firework {
public:
    Firework(float x, float groundY, float hue);

    void update();
    bool done() const { return state_ == FWState::DONE; }

    std::vector<Particle> particles;

private:
    void burst();
    void willow();
    void ring();
    void chrysanthemum();
    void glitter();

    void explode();                   // dispatches to above

    FWState   state_;
    FWPattern pattern_;
    float     hue_;
    int       rocketIdx_ = 0;
};
