#pragma once
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline float randf(float lo = 0.0f, float hi = 1.0f) {
    return lo + (hi - lo) * (float)rand() / (float)RAND_MAX;
}
inline int randi(int lo, int hi) {
    return lo + rand() % (hi - lo + 1);
}

struct Vec2 {
    float x = 0, y = 0;
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2  operator+ (const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2  operator- (const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2  operator* (float t)       const { return {x*t,   y*t};   }
    Vec2& operator+=(const Vec2& o)       { x+=o.x; y+=o.y; return *this; }
    float len() const { return std::sqrt(x*x + y*y); }
};
