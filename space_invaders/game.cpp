#include "game.hpp"
#include "input.hpp"
#include <cstdlib>
#include <cmath>
#include <algorithm>

const std::vector<Bullet>&    Game::bullets()    const { return bullets_; }
const std::vector<Explosion>& Game::explosions() const { return expl_; }

static int randInt(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────
int Game::alive() const {
    int n = 0;
    for (int r = 0; r < EROWS; ++r)
        for (int c = 0; c < ECOLS; ++c)
            if (e_[r][c].alive) ++n;
    return n;
}

int Game::lowestEnemyRow() const {
    for (int r = EROWS-1; r >= 0; --r)
        for (int c = 0; c < ECOLS; ++c)
            if (e_[r][c].alive) return r;
    return 0;
}

int Game::stepInterval() const {
    // Speed up as enemies die: starts at 22 frames, drops to 4
    int n = alive();
    return std::max(4, 4 + (n * 18) / (EROWS * ECOLS));
}

void Game::boom(int x, int y) {
    for (auto& ex : expl_) {
        if (!ex.active) { ex = {x, y, 0, 10, true}; return; }
    }
    expl_.push_back({x, y, 0, 10, true});
}

// ─────────────────────────────────────────────────────────────
//  Constructor / Reset
// ─────────────────────────────────────────────────────────────
Game::Game(int w, int h) : W_(w), H_(h), hi_(0) {
    reset(true);
}

void Game::reset(bool fullReset) {
    state_  = fullReset ? GState::TITLE : GState::PLAYING;
    frame_  = 0;
    fireCd_ = 0;
    hitFlash_  = 0;
    deadTimer_ = 0;
    fdir_      = 1;
    stepTimer_ = stepInterval();

    if (fullReset) {
        score_  = 0;
        lives_  = 3;
        wave_   = 1;
    }

    // Place enemies: formation starts top-left
    fx_      = 3.0f;
    fy_      = 3.0f;
    for (int r = 0; r < EROWS; ++r)
        for (int c = 0; c < ECOLS; ++c)
            e_[r][c].alive = true;

    // Player in center bottom
    px_ = W_ / 2.0f;

    // Shoot timer: wave affects frequency (shorter = harder)
    shootTimer_ = std::max(20, 60 - wave_ * 5);

    bullets_.clear();
    expl_.clear();
}

// ─────────────────────────────────────────────────────────────
//  Enemy movement
// ─────────────────────────────────────────────────────────────
void Game::stepEnemies() {
    float formW = (ECOLS - 1) * ESPX + 2;  // approx formation width

    // Try to move horizontally
    float nx = fx_ + fdir_;
    bool  hitWall = (nx < 1.0f) || (nx + formW >= W_ - 1);

    if (hitWall) {
        fdir_  = -fdir_;
        fy_   += 1.0f;   // march one row closer to player
    } else {
        fx_ = nx;
    }

    // Enemy reaches player zone → game over
    float bottomY = fy_ + (lowestEnemyRow() * ESPY) + 1;
    if (bottomY >= H_ - 3) {
        lives_ = 0;
        state_ = GState::GAME_OVER;
    }
}

// ─────────────────────────────────────────────────────────────
//  Bullet update
// ─────────────────────────────────────────────────────────────
void Game::updateBullets() {
    for (auto& b : bullets_) {
        if (!b.active) continue;
        b.y += b.fromPlayer ? -1.0f : 1.0f;
        if (b.y < 0 || b.y >= H_) b.active = false;
    }
}

// ─────────────────────────────────────────────────────────────
//  Enemy shooting
// ─────────────────────────────────────────────────────────────
void Game::enemyShoot() {
    // Pick a random alive enemy in the bottom-most alive row of each column
    for (int c = 0; c < ECOLS; ++c) {
        for (int r = EROWS-1; r >= 0; --r) {
            if (!e_[r][c].alive) continue;
            if (randInt(0, ECOLS * 4) == 0) {  // low chance per column
                float bx = fx_ + c * ESPX + 1;
                float by = fy_ + r * ESPY + 1;
                // Find or reuse an inactive bullet
                for (auto& b : bullets_) {
                    if (!b.active) { b = {bx, by, false, true}; goto done; }
                }
                bullets_.push_back({bx, by, false, true});
                done: break;
            }
            break; // only bottom-most alive enemy per column can shoot
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  Collision detection
// ─────────────────────────────────────────────────────────────
void Game::collide() {
    for (auto& b : bullets_) {
        if (!b.active) continue;
        int bx = (int)std::round(b.x);
        int by = (int)std::round(b.y);

        if (b.fromPlayer) {
            // vs enemies
            for (int r = 0; r < EROWS; ++r) {
                for (int c = 0; c < ECOLS; ++c) {
                    if (!e_[r][c].alive) continue;
                    int ex = (int)(fx_ + c * ESPX);
                    int ey = (int)(fy_ + r * ESPY);
                    if (bx >= ex && bx <= ex+2 && by == ey) {
                        e_[r][c].alive = false;
                        b.active = false;
                        score_ += ESCORE[r] * wave_;
                        if (score_ > hi_) hi_ = score_;
                        boom(ex+1, ey);
                        if (alive() == 0) { state_ = GState::WIN; return; }
                        goto next_bullet;
                    }
                }
            }
        } else {
            // vs player
            if (hitFlash_ == 0) {
                int px = (int)std::round(px_);
                if (bx >= px-1 && bx <= px+1 && by == H_-2) {
                    b.active = false;
                    boom(px, H_-2);
                    --lives_;
                    hitFlash_  = 30;
                    deadTimer_ = 60;
                    if (lives_ <= 0) { state_ = GState::GAME_OVER; return; }
                    // remove all enemy bullets after player hit
                    for (auto& bb : bullets_) if (!bb.fromPlayer) bb.active = false;
                }
            }
        }
        next_bullet:;
    }
}

// ─────────────────────────────────────────────────────────────
//  Main update (called every frame)
// ─────────────────────────────────────────────────────────────
void Game::update(int key) {
    ++frame_;

    // ── Title screen ──────────────────────────────────────────
    if (state_ == GState::TITLE) {
        if (key == KEY_SPACE || key == KEY_ENTER) reset(false);
        return;
    }

    // ── Win → next wave ───────────────────────────────────────
    if (state_ == GState::WIN) {
        if (key == KEY_SPACE || key == KEY_ENTER || frame_ % 80 == 0) {
            ++wave_;
            reset(false);
        }
        return;
    }

    // ── Game Over ─────────────────────────────────────────────
    if (state_ == GState::GAME_OVER) {
        if (key == KEY_R) reset(true);
        return;
    }

    // ── Playing ───────────────────────────────────────────────

    // Player temporarily invincible after being hit
    if (hitFlash_ > 0)  --hitFlash_;
    if (deadTimer_ > 0) { --deadTimer_; return; }

    // ── Input ─────────────────────────────────────────────────
    if (key == KEY_LEFT  || key == 'a') px_ = std::max(1.0f, px_ - 1.0f);
    if (key == KEY_RIGHT || key == 'd') px_ = std::min((float)W_-2, px_ + 1.0f);

    if ((key == KEY_SPACE || key == 'w' || key == KEY_UP) && fireCd_ == 0) {
        for (auto& b : bullets_) {
            if (!b.active) { b = {px_, (float)(H_-3), true, true}; goto fired; }
        }
        bullets_.push_back({px_, (float)(H_-3), true, true});
        fired:
        fireCd_ = 12;
    }
    if (fireCd_ > 0) --fireCd_;

    // ── Enemy step ────────────────────────────────────────────
    if (--stepTimer_ <= 0) {
        stepTimer_ = stepInterval();
        stepEnemies();
    }

    // ── Enemy shooting ────────────────────────────────────────
    if (--shootTimer_ <= 0) {
        shootTimer_ = std::max(15, 50 - wave_ * 4);
        enemyShoot();
    }

    // ── Physics / collisions ──────────────────────────────────
    updateBullets();
    collide();

    // ── Explosions ────────────────────────────────────────────
    for (auto& ex : expl_) {
        if (ex.active) ++ex.frame;
        if (ex.frame >= ex.duration) ex.active = false;
    }
}
