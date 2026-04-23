#include "renderer.hpp"
#include <iostream>
#include <cstring>
#include <cmath>

// ── Enemy sprites (3 chars each, by row) ─────────────────────
static const char* ESPRITE[EROWS] = { "/^\\", "oOo", "(o)", "[.]" };

// ── Enemy colors ──────────────────────────────────────────────
static const Color ECOL[EROWS] = {
    {255,  80,  80},   // row 0 → red
    {255, 160,  40},   // row 1 → orange
    {100, 230, 100},   // row 2 → green
    {80,  180, 255},   // row 3 → cyan
};

static const Color PLAYER_COL  { 60, 220, 255};
static const Color PLAYER_HIT  {255,  60,  60};
static const Color PBULLET_COL {255, 255, 120};
static const Color EBULLET_COL {255,  60,  60};
static const Color STAR_COL    {120, 130, 160};
static const Color UI_COL      {255, 220,  50};
static const Color EXPL_COL    {255, 140,  20};
static const Color DIM         { 80,  80, 120};

// ─────────────────────────────────────────────────────────────
Renderer::Renderer(int w, int h) : W_(w), H_(h) {
    buf_.assign(h, std::vector<Cell>(w));
}

void Renderer::clear() {
    for (auto& row : buf_)
        for (auto& c : row) { c.ch = ' '; c.col = {4, 4, 18}; }
}

void Renderer::put(int x, int y, char ch, Color col) {
    if (x < 0 || x >= W_ || y < 0 || y >= H_) return;
    buf_[y][x] = {ch, col};
}

void Renderer::text(int x, int y, const char* s, Color col) {
    for (int i = 0; s[i]; ++i) put(x + i, y, s[i], col);
}

// ─────────────────────────────────────────────────────────────
void Renderer::draw(const Game& g) {
    clear();

    switch (g.state()) {
        case GState::TITLE:    drawTitle(g);    break;
        case GState::PLAYING:  drawPlaying(g);  break;
        case GState::WIN:      drawPlaying(g); drawWin(g);     break;
        case GState::GAME_OVER:drawPlaying(g); drawGameOver(g);break;
        default: break;
    }
    flush(g);
}

// ─────────────────────────────────────────────────────────────
//  TITLE
// ─────────────────────────────────────────────────────────────
void Renderer::drawTitle(const Game& g) {
    int cx = g.W() / 2;
    int cy = g.H() / 2 - 6;

    // Big title art
    const char* art[] = {
        " ____  ____   _    ____ _____   ___ _   ___     ___    ____  _____ ____  ____",
        "/ ___||  _ \\ / \\  / ___| ____| |_ _| \\ | \\ \\   / / \\  |  _ \\| ____|  _ \\/ ___|",
        "\\___ \\| |_) / _ \\| |   |  _|    | ||  \\| |\\ \\ / / _ \\ | | | |  _| | |_) \\___ \\",
        " ___) |  __/ ___ \\ |___| |___   | || |\\  | \\ V / ___ \\| |_| | |___|  _ < ___) |",
        "|____/|_| /_/   \\_\\____|_____| |___|_| \\_|  \\_/_/   \\_\\____/|_____|_| \\_\\____/",
    };

    Color titleCol = (g.frame() / 8 % 2 == 0) ? Color{255,220,50} : Color{255,100,50};
    for (int i = 0; i < 5; ++i)
        text(1, cy + i, art[i], titleCol);

    text(cx - 12, cy + 7, "[ SPACE ] or [ ENTER ] to start", {200, 200, 255});
    text(cx - 10, cy + 9, "Arrow Keys / WASD  to move", {150, 150, 200});
    text(cx - 10, cy +10, "SPACE / W  to shoot", {150, 150, 200});
    text(cx - 10, cy +11, "Q  to quit", {150, 150, 200});

    text(cx - 8, cy + 13, "/^\\  oOo  (o)  [.]", DIM);

    text(cx - 12, cy + 15, ("HI SCORE: " + std::to_string(g.hi())).c_str(), UI_COL);
}

// ─────────────────────────────────────────────────────────────
//  PLAYING
// ─────────────────────────────────────────────────────────────
void Renderer::drawPlaying(const Game& g) {
    // ── Starfield (deterministic: hash frame+pos) ─────────────
    static const int STARS = 30;
    for (int i = 0; i < STARS; ++i) {
        int sx = (i * 137 + 7)  % g.W();
        int sy = (i * 97  + 13) % (g.H() - 3);
        char sc = (i % 3 == 0) ? '*' : ((i % 3 == 1) ? '+' : '.');
        Color scol = ((g.frame() / (4 + i % 5)) % 2 == 0) ? STAR_COL : Color{40,40,70};
        put(sx, sy, sc, scol);
    }

    // ── Enemies ───────────────────────────────────────────────
    auto& enemies = g.enemies();
    for (int r = 0; r < EROWS; ++r) {
        for (int c = 0; c < ECOLS; ++c) {
            if (!enemies[r][c].alive) continue;
            int x = (int)g.formX() + c * ESPX;
            int y = (int)g.formY() + r * ESPY;
            // Animate: alternate between sprite and variant each 15 frames
            bool alt = (g.frame() / 15 % 2 == 1);
            const char* spr = ESPRITE[r];
            for (int i = 0; i < 3; ++i) {
                char ch = spr[i];
                if (alt && i == 1) ch = (ch == 'o') ? '0' : (ch == '^') ? 'v' : ch;
                put(x + i, y, ch, ECOL[r]);
            }
        }
    }

    // ── Bullets ───────────────────────────────────────────────
    for (auto& b : g.bullets()) {
        if (!b.active) continue;
        char  ch  = b.fromPlayer ? '|' : 'v';
        Color col = b.fromPlayer ? PBULLET_COL : EBULLET_COL;
        put((int)std::round(b.x), (int)std::round(b.y), ch, col);
    }

    // ── Explosions ────────────────────────────────────────────
    const char* exframes[] = { "*", "+", "x", ".", " " };
    for (auto& ex : g.explosions()) {
        if (!ex.active) continue;
        int fi = (ex.frame * 5) / ex.duration;
        fi = std::min(fi, 4);
        float fade = 1.0f - (float)ex.frame / ex.duration;
        Color col{ (uint8_t)(255*fade), (uint8_t)(140*fade), 0 };
        put(ex.x,   ex.y,   exframes[fi][0], col);
        put(ex.x-1, ex.y,   exframes[fi][0], col);
        put(ex.x+1, ex.y,   exframes[fi][0], col);
        put(ex.x,   ex.y-1, exframes[fi][0], col);
    }

    // ── Player ship ───────────────────────────────────────────
    int px = (int)std::round(g.playerX());
    int py = g.H() - 2;
    Color pcol = g.playerHit() ? PLAYER_HIT : PLAYER_COL;
    bool  blink = g.playerHit() && (g.frame() / 3 % 2 == 0);
    if (!blink) {
        put(px-1, py, '\\', pcol);
        put(px,   py, '^',  pcol);
        put(px+1, py, '/',  pcol);
        put(px,   py+1, '|', pcol);   // engine exhaust
    }

    // ── Ground line ───────────────────────────────────────────
    for (int x = 0; x < g.W(); ++x)
        put(x, g.H()-1, '-', {50, 50, 100});

    drawHUD(g);
}

// ─────────────────────────────────────────────────────────────
//  HUD
// ─────────────────────────────────────────────────────────────
void Renderer::drawHUD(const Game& g) {
    // Lives as ship icons
    std::string lifeStr = "LIVES: ";
    for (int i = 0; i < g.lives(); ++i) lifeStr += "\\^/ ";
    text(1, 0, lifeStr.c_str(), PLAYER_COL);

    text(g.W()/2 - 8, 0, ("SCORE: " + std::to_string(g.score())).c_str(), UI_COL);
    text(g.W() - 20,  0, ("HI: " + std::to_string(g.hi())).c_str(), {200,200,80});
    text(g.W() - 10,  0, ("W" + std::to_string(g.wave())).c_str(), {150,80,255});
}

// ─────────────────────────────────────────────────────────────
//  WIN / GAME OVER overlays
// ─────────────────────────────────────────────────────────────
void Renderer::drawWin(const Game& g) {
    int cx = g.W()/2, cy = g.H()/2 - 2;
    Color c = (g.frame() / 6 % 2 == 0) ? Color{100,255,100} : Color{255,255,50};
    text(cx-8, cy,   "*** WAVE CLEAR! ***", c);
    text(cx-12, cy+2, ("Bonus: +" + std::to_string(g.wave()*500)).c_str(), UI_COL);
    text(cx-12, cy+4, "[ SPACE ] = Next Wave", {180,180,255});
}

void Renderer::drawGameOver(const Game& g) {
    int cx = g.W()/2, cy = g.H()/2 - 3;
    Color c = (g.frame() / 5 % 2 == 0) ? Color{255,60,60} : Color{200,0,0};
    text(cx-8,  cy,   "*** GAME OVER ***", c);
    text(cx-10, cy+2, ("Final Score: " + std::to_string(g.score())).c_str(), UI_COL);
    if (g.score() >= g.hi() && g.score() > 0)
        text(cx-6, cy+3, "NEW HI SCORE!", {255,220,0});
    text(cx-8,  cy+5, "[ R ] = Restart", {180,180,255});
    text(cx-8,  cy+6, "[ Q ] = Quit",    {180,180,255});
}

// ─────────────────────────────────────────────────────────────
//  Flush buffer to terminal
// ─────────────────────────────────────────────────────────────
void Renderer::flush(const Game&) {
    std::string out;
    out.reserve(W_ * H_ * 18);
    out += "\033[H";

    Color last{0, 0, 0};
    bool  newLine = true;

    for (int y = 0; y < H_; ++y) {
        for (int x = 0; x < W_; ++x) {
            const Cell& c = buf_[y][x];
            if (newLine || c.col != last) {
                out += c.col.fg();
                last    = c.col;
                newLine = false;
            }
            out += c.ch;
        }
        out += "\033[0m\n";
        newLine = true;
    }
    std::cout << out << std::flush;
}
