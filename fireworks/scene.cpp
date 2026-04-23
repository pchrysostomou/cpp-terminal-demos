#include "scene.hpp"
#include <algorithm>

Scene::Scene(int width, int height)
    : W_(width), H_(height)
{
    initStars(45);
    nextSpawn_ = randi(5, 20);
}

void Scene::update() {
    ++frame_;

    // ── Finale trigger (after 30 seconds @ 20fps = 600 frames) ──
    if (!finaleActive_ && frame_ >= 600) {
        finaleActive_ = true;
        finaleStart_  = frame_;
    }

    // ── Spawn logic ──────────────────────────────────────────────
    if (frame_ >= nextSpawn_) {
        if (finaleActive_) {
            // Rapid-fire during finale
            int burst = randi(3, 6);
            spawnFirework(burst);
            nextSpawn_ = frame_ + randi(8, 20);
        } else {
            int count = (randf() < 0.25f) ? 2 : 1;
            spawnFirework(count);
            nextSpawn_ = frame_ + randi(28, 55);
        }
    }

    // ── Update ───────────────────────────────────────────────────
    for (auto& fw : fireworks) fw.update();

    fireworks.erase(
        std::remove_if(fireworks.begin(), fireworks.end(),
                       [](const Firework& f){ return f.done(); }),
        fireworks.end());

    for (auto& s : stars) s.update();
}

void Scene::spawnFirework(int count) {
    for (int i = 0; i < count; ++i) {
        float x   = randf(W_ * 0.08f, W_ * 0.92f);
        float hue = randf(0.0f, 1.0f);
        fireworks.emplace_back(x, (float)(H_ - 1), hue);
    }
}

void Scene::initStars(int n) {
    for (int i = 0; i < n; ++i) {
        Particle s;
        s.pos   = {randf(0.0f, (float)W_), randf(0.0f, (float)H_ * 0.75f)};
        s.vel   = {0, 0};
        s.life  = randf(0.2f, 1.0f);
        s.decay = randf(0.004f, 0.018f);
        s.color = Color{180, 190, 215};
        s.type  = PType::STAR;
        stars.push_back(s);
    }
}
