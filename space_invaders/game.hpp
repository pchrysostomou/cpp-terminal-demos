#pragma once
#include <vector>

// ── Grid dimensions ───────────────────────────────────────────
static constexpr int ECOLS  = 11;   // enemy columns
static constexpr int EROWS  =  4;   // enemy rows
static constexpr int ESPX   =  4;   // horizontal spacing
static constexpr int ESPY   =  2;   // vertical spacing

// ── Scoring per row (top → bottom) ────────────────────────────
static constexpr int ESCORE[EROWS] = { 30, 20, 20, 10 };

// ── Entity structs ────────────────────────────────────────────
struct Enemy {
    bool alive = true;
};

struct Bullet {
    float x, y;
    bool  fromPlayer;
    bool  active = false;
};

struct Explosion {
    int  x, y;
    int  frame    = 0;
    int  duration = 10;
    bool active   = false;
};

// ── Game state ────────────────────────────────────────────────
enum class GState { TITLE, PLAYING, DEAD, GAME_OVER, WIN };

class Game {
public:
    Game(int w, int h);

    void update(int key);
    void reset(bool fullReset = true);

    // ── Accessors for Renderer ────────────────────────────────
    GState state()    const { return state_; }
    int    score()    const { return score_; }
    int    hi()       const { return hi_; }
    int    lives()    const { return lives_; }
    int    wave()     const { return wave_; }
    int    frame()    const { return frame_; }
    int    W()        const { return W_; }
    int    H()        const { return H_; }
    float  playerX()  const { return px_; }
    bool   playerHit()const { return hitFlash_ > 0; }

    float formX()     const { return fx_; }
    float formY()     const { return fy_; }

    const Enemy    (&enemies() const)[EROWS][ECOLS] { return e_; }
    const std::vector<Bullet>&    bullets()    const;
    const std::vector<Explosion>& explosions() const;

private:
    void stepEnemies();
    void updateBullets();
    void collide();
    void enemyShoot();
    void boom(int x, int y);
    int  alive()          const;
    int  lowestEnemyRow() const;
    int  stepInterval()   const;

    int    W_, H_;
    GState state_;
    int    score_, hi_, lives_, wave_, frame_;

    float px_;             // player x (center)
    int   fireCd_ = 0;     // fire cooldown frames
    int   hitFlash_  = 0;  // frames of player hit flash
    int   deadTimer_ = 0;  // frames between death and respawn

    Enemy   e_[EROWS][ECOLS];
    float   fx_, fy_;      // formation top-left (terminal coords)
    int     fdir_;         // +1 right, -1 left
    int     stepTimer_;    // countdown to next enemy step
    int     shootTimer_;   // countdown to next enemy shot

    std::vector<Bullet>    bullets_;
    std::vector<Explosion> expl_;
};
