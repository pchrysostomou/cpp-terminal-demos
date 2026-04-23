#pragma once
#include <vector>
#include "firework.hpp"

class Scene {
public:
    Scene(int width, int height);

    void update();
    int  frame()  const { return frame_; }
    int  width()  const { return W_; }
    int  height() const { return H_; }

    std::vector<Firework> fireworks;
    std::vector<Particle> stars;

private:
    void spawnFirework(int count = 1);
    void initStars(int n);

    int W_, H_;
    int frame_         = 0;
    int nextSpawn_     = 0;
    int finaleStart_   = -1;    // frame when finale begins
    bool finaleActive_ = false;
};
