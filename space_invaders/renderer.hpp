#pragma once
#include "game.hpp"
#include "color.hpp"
#include <string>
#include <vector>

class Renderer {
public:
    Renderer(int w, int h);
    void draw(const Game& g);

private:
    struct Cell { char ch = ' '; Color col; };

    int W_, H_;
    std::vector<std::vector<Cell>> buf_;

    void clear();
    void put(int x, int y, char ch, Color col);
    void text(int x, int y, const char* s, Color col);
    void flush(const Game& g);

    void drawTitle (const Game& g);
    void drawPlaying(const Game& g);
    void drawWin   (const Game& g);
    void drawGameOver(const Game& g);
    void drawHUD   (const Game& g);
};
