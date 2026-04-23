#pragma once
#include <string>
#include <vector>
#include "scene.hpp"

class Renderer {
public:
    Renderer(int width, int height);
    void render(const Scene& sc, double fps);

private:
    struct Cell {
        std::string glyph  = " ";
        Color       color  = {3, 3, 12};
        int         zorder = 0;
    };

    int W_, H_;
    std::vector<std::vector<Cell>> buf_;

    void clearBuf();
    void plot(const Particle& p, int z);
    void flush(const Scene& sc, double fps);
};
