// ╔══════════════════════════════════════════════════════════════╗
// ║         C++ TERMINAL RAY TRACER  —  ASCII Art Edition        ║
// ║  Phong · Reflections · Shadows · Anti-Aliasing · Animation   ║
// ╠══════════════════════════════════════════════════════════════╣
// ║  Compile:  g++ -O2 -std=c++17 -o raytracer raytracer.cpp    ║
// ║  Run:      ./raytracer                                       ║
// ║  Quit:     Ctrl+C                                            ║
// ╚══════════════════════════════════════════════════════════════╝

#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <csignal>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ═══════════════════════════════════════════════════════════════
//  MATH  —  Vec3
// ═══════════════════════════════════════════════════════════════

struct Vec3 {
    double x{}, y{}, z{};

    Vec3() = default;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(double t)      const { return {x*t,   y*t,   z*t}; }
    Vec3 operator*(const Vec3& o) const { return {x*o.x, y*o.y, z*o.z}; }
    Vec3 operator/(double t)      const { return {x/t,   y/t,   z/t}; }
    Vec3 operator-()              const { return {-x, -y, -z}; }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }

    double dot  (const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3   cross(const Vec3& o) const {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
    }
    double len()  const { return std::sqrt(x*x + y*y + z*z); }
    Vec3   norm() const { double l = len(); return l > 1e-12 ? *this/l : Vec3{}; }
    Vec3   reflect(const Vec3& n) const { return *this - n * (2.0 * dot(n)); }
    Vec3   clamp01() const {
        return {std::min(1.0,std::max(0.0,x)),
                std::min(1.0,std::max(0.0,y)),
                std::min(1.0,std::max(0.0,z))};
    }
};

Vec3 operator*(double t, const Vec3& v) { return v * t; }

// ═══════════════════════════════════════════════════════════════
//  RAY
// ═══════════════════════════════════════════════════════════════

struct Ray {
    Vec3 origin, dir;
    Ray(Vec3 o, Vec3 d) : origin(o), dir(d.norm()) {}
    Vec3 at(double t) const { return origin + dir * t; }
};

// ═══════════════════════════════════════════════════════════════
//  MATERIAL
// ═══════════════════════════════════════════════════════════════

struct Material {
    Vec3   color;
    double ambient, diffuse, specular, shininess, reflectivity;
    bool   is_emissive;
    Vec3   emissive;

    Material()
        : color(1,1,1), ambient(0.1), diffuse(0.9), specular(0.5),
          shininess(32), reflectivity(0.0), is_emissive(false), emissive(0,0,0) {}

    Material(Vec3 col, double amb, double diff, double spec,
             double shin, double refl,
             bool emit = false, Vec3 emcol = Vec3{})
        : color(col), ambient(amb), diffuse(diff), specular(spec),
          shininess(shin), reflectivity(refl), is_emissive(emit), emissive(emcol) {}
};

// ═══════════════════════════════════════════════════════════════
//  HIT RECORD
// ═══════════════════════════════════════════════════════════════

struct Hit {
    bool     valid {false};
    double   t     {1e18};
    Vec3     point, normal;
    Material mat;
};

// ═══════════════════════════════════════════════════════════════
//  SPHERE
// ═══════════════════════════════════════════════════════════════

struct Sphere {
    Vec3     center;
    double   radius;
    Material mat;

    Hit intersect(const Ray& ray) const {
        Vec3   oc = ray.origin - center;
        double a  = ray.dir.dot(ray.dir);
        double b  = 2.0 * oc.dot(ray.dir);
        double c  = oc.dot(oc) - radius*radius;
        double D  = b*b - 4*a*c;
        if (D < 0) return {};

        double sq = std::sqrt(D);
        double t  = (-b - sq) / (2*a);
        if (t < 1e-4) t = (-b + sq) / (2*a);
        if (t < 1e-4) return {};

        Hit h;
        h.valid  = true;
        h.t      = t;
        h.point  = ray.at(t);
        h.normal = (h.point - center).norm();
        h.mat    = mat;
        return h;
    }
};

// ═══════════════════════════════════════════════════════════════
//  PLANE  (infinite, checkerboard pattern)
// ═══════════════════════════════════════════════════════════════

struct Plane {
    Vec3     point, normal;
    Material mat;

    Hit intersect(const Ray& ray) const {
        double denom = normal.dot(ray.dir);
        if (std::abs(denom) < 1e-8) return {};
        double t = (point - ray.origin).dot(normal) / denom;
        if (t < 1e-4) return {};

        Hit h;
        h.valid  = true;
        h.t      = t;
        h.point  = ray.at(t);
        h.normal = normal;
        h.mat    = mat;

        // Checkerboard — safe modulo for negative coords
        int ix = (int)std::floor(h.point.x * 0.7);
        int iz = (int)std::floor(h.point.z * 0.7);
        bool white = (((ix % 2) + 2) % 2) == (((iz % 2) + 2) % 2);
        h.mat.color = white ? Vec3{0.92,0.92,0.92} : Vec3{0.12,0.12,0.12};
        return h;
    }
};

// ═══════════════════════════════════════════════════════════════
//  LIGHT
// ═══════════════════════════════════════════════════════════════

struct Light {
    Vec3   pos, color;
    double intensity{1.0};
};

// ═══════════════════════════════════════════════════════════════
//  SCENE
// ═══════════════════════════════════════════════════════════════

class Scene {
public:
    std::vector<Sphere> spheres;
    std::vector<Plane>  planes;
    std::vector<Light>  lights;

    Hit trace(const Ray& ray) const {
        Hit best;
        for (auto& s : spheres) { auto h = s.intersect(ray); if (h.valid && h.t < best.t) best = h; }
        for (auto& p : planes)  { auto h = p.intersect(ray); if (h.valid && h.t < best.t) best = h; }
        return best;
    }

    bool inShadow(const Vec3& pt, const Light& L) const {
        Vec3   dir  = L.pos - pt;
        double dist = dir.len();
        Ray    sr{pt, dir};
        Hit    h = trace(sr);
        return h.valid && h.t < dist - 1e-4;
    }

    Vec3 shade(const Hit& h, const Ray& ray, int depth) const {
        if (!h.valid) {
            // Sky gradient
            double t = 0.5 * (ray.dir.y + 1.0);
            return Vec3{0.06,0.06,0.20} * (1-t) + Vec3{0.28,0.40,0.80} * t;
        }
        if (h.mat.is_emissive) return h.mat.emissive;

        Vec3 col{};

        for (auto& L : lights) {
            // Ambient
            col += h.mat.color * L.color * (h.mat.ambient * L.intensity);

            if (!inShadow(h.point, L)) {
                Vec3 toL   = (L.pos - h.point).norm();
                Vec3 toCam = (-ray.dir).norm();
                Vec3 half  = (toL + toCam).norm();

                double diff = std::max(0.0, h.normal.dot(toL));
                double spec = std::pow(std::max(0.0, h.normal.dot(half)), h.mat.shininess);

                col += h.mat.color * L.color * (h.mat.diffuse * diff * L.intensity);
                col += L.color * (h.mat.specular * spec * L.intensity);
            }
        }

        // Recursive reflections
        if (depth > 0 && h.mat.reflectivity > 0.001) {
            Vec3 rdir = ray.dir.reflect(h.normal);
            Ray  rray{h.point, rdir};
            Hit  rh   = trace(rray);
            Vec3 rcol = shade(rh, rray, depth - 1);
            col = col * (1.0 - h.mat.reflectivity) + rcol * h.mat.reflectivity;
        }

        return col.clamp01();
    }
};

// ═══════════════════════════════════════════════════════════════
//  CAMERA
// ═══════════════════════════════════════════════════════════════

class Camera {
    Vec3 pos_, ll_, horiz_, vert_;

public:
    Camera(Vec3 from, Vec3 at, Vec3 up, double fov_deg, double aspect) {
        double theta  = fov_deg * M_PI / 180.0;
        double half_h = std::tan(theta / 2.0);
        double half_w = aspect * half_h;

        Vec3 w = (from - at).norm();
        Vec3 u = up.cross(w).norm();
        Vec3 v = w.cross(u);

        pos_   = from;
        ll_    = from - u*half_w - v*half_h - w;
        horiz_ = u * (2*half_w);
        vert_  = v * (2*half_h);
    }

    Ray getRay(double s, double t) const {
        return {pos_, ll_ + horiz_*s + vert_*t - pos_};
    }
};

// ═══════════════════════════════════════════════════════════════
//  RENDERER  —  ASCII + ANSI true color
// ═══════════════════════════════════════════════════════════════

class Renderer {
    int W_, H_, MAX_DEPTH_;

    static std::string rgb(int r, int g, int b) {
        return "\033[38;2;" + std::to_string(r) + ";"
                            + std::to_string(g) + ";"
                            + std::to_string(b) + "m";
    }

    char toAscii(double b) {
        static const std::string ramp =
            " .`-_':,;^=+/|)\\<>ivxclrs{*}I?!][taeo7zjLu#JCwfy325Fp6mqSgVd4EXPGZbYkOA&8U$@MNW0B";
        int idx = (int)(b * (double)(ramp.size() - 1));
        idx = std::max(0, std::min((int)ramp.size()-1, idx));
        return ramp[idx];
    }

public:
    Renderer(int w, int h, int d = 4) : W_(w), H_(h), MAX_DEPTH_(d) {}

    void render(const Scene& sc, const Camera& cam) {
        std::ostringstream out;
        out << "\033[H";   // cursor home (redraws in-place)

        for (int j = H_-1; j >= 0; --j) {
            for (int i = 0; i < W_; ++i) {
                // 2×2 supersampling for anti-aliasing
                Vec3 col{};
                for (int sj = 0; sj < 2; ++sj) {
                    for (int si = 0; si < 2; ++si) {
                        double u = (i + 0.25 + si*0.5) / (W_ - 1);
                        double v = (j + 0.25 + sj*0.5) / (H_ - 1);
                        Ray r = cam.getRay(u, v);
                        Hit h = sc.trace(r);
                        col += sc.shade(h, r, MAX_DEPTH_);
                    }
                }
                col = (col / 4.0).clamp01();

                int ir = (int)(col.x * 255);
                int ig = (int)(col.y * 255);
                int ib = (int)(col.z * 255);

                // Perceptual luminance → ASCII character
                double lum = 0.2126*col.x + 0.7152*col.y + 0.0722*col.z;
                char   ch  = toAscii(lum);

                // Two chars wide (terminal cells are taller than wide)
                out << rgb(ir,ig,ib) << ch << ch;
            }
            out << "\033[0m\n";
        }
        std::cout << out.str() << std::flush;
    }
};

// ═══════════════════════════════════════════════════════════════
//  HSV → RGB helper
// ═══════════════════════════════════════════════════════════════

Vec3 hsvToRgb(double h, double s, double v) {
    int    hi = (int)(h * 6) % 6;
    double f  = h*6 - std::floor(h*6);
    double p  = v*(1-s), q = v*(1-f*s), t = v*(1-(1-f)*s);
    switch(hi) {
        case 0: return {v,t,p};
        case 1: return {q,v,p};
        case 2: return {p,v,t};
        case 3: return {p,q,v};
        case 4: return {t,p,v};
        default:return {v,p,q};
    }
}

// ═══════════════════════════════════════════════════════════════
//  SCENE BUILDER  (animated by time t)
// ═══════════════════════════════════════════════════════════════

Scene buildScene(double t) {
    Scene sc;

    // ── Floor ──────────────────────────────────────────────────
    sc.planes.push_back(Plane{
        {0,-0.15,0}, {0,1,0},
        Material{Vec3{1,1,1}, 0.05, 0.85, 0.35, 8, 0.28}
    });

    // ── Central gold sphere ────────────────────────────────────
    sc.spheres.push_back(Sphere{
        {0, 1.15, 0}, 0.9,
        Material{Vec3{1.0,0.76,0.05}, 0.04, 0.55, 2.8, 320, 0.6}
    });

    // ── Three orbiting main spheres ────────────────────────────
    Vec3 baseColors[3] = {
        {0.95, 0.12, 0.12},
        {0.12, 0.35, 0.95},
        {0.12, 0.88, 0.28}
    };
    for (int i = 0; i < 3; ++i) {
        double angle = t + i * (2.0*M_PI/3.0);
        Vec3 center{std::cos(angle)*2.3, 0.55, std::sin(angle)*2.3};
        sc.spheres.push_back(Sphere{
            center, 0.52,
            Material{baseColors[i], 0.04, 0.8, 2.0, 128, 0.38}
        });
    }

    // ── Left mirror sphere ─────────────────────────────────────
    sc.spheres.push_back(Sphere{
        {-4.0, 0.75, -0.5}, 0.75,
        Material{Vec3{0.88,0.90,0.94}, 0.02, 0.05, 2.5, 512, 0.94}
    });

    // ── Right purple sphere ────────────────────────────────────
    sc.spheres.push_back(Sphere{
        {4.0, 0.75, -0.5}, 0.75,
        Material{Vec3{0.62,0.10,0.92}, 0.04, 0.78, 1.8, 96, 0.32}
    });

    // ── Small rainbow dancing spheres around the central one ───
    for (int i = 0; i < 7; ++i) {
        double angle = t*1.7 + i * (2.0*M_PI/7.0);
        double y     = 1.95 + std::sin(t*2.4 + i * 1.1) * 0.22;
        Vec3   pos{std::cos(angle)*1.25, y, std::sin(angle)*1.25};
        Vec3   col = hsvToRgb((double)i/7.0, 1.0, 1.0);
        sc.spheres.push_back(Sphere{
            pos, 0.17,
            Material{col, 0.05, 0.82, 1.6, 80, 0.25}
        });
    }

    // ── Glowing emissive sphere (back wall) ────────────────────
    double glow_r = 0.5 + 0.1 * std::sin(t * 3.0);
    Vec3   glow_c{0.9 + 0.1*std::sin(t), 0.5 + 0.2*std::cos(t*0.7), 0.1};
    sc.spheres.push_back(Sphere{
        {0, 0.35, -6.0}, glow_r,
        Material{glow_c, 0,0,0,0,0, true, glow_c * 1.4}
    });

    // ── Small satellite of the emissive sphere ─────────────────
    {
        double a2 = t * 2.5;
        Vec3   p2{std::cos(a2)*1.1, 0.35 + std::sin(a2*1.3)*0.3, -6.0 + std::sin(a2)*1.1};
        sc.spheres.push_back(Sphere{
            p2, 0.18,
            Material{Vec3{0.2,0.6,1.0}, 0.05, 0.8, 1.5, 64, 0.3}
        });
    }

    // ── Lights ────────────────────────────────────────────────
    sc.lights.push_back(Light{{5,  8, 5},   {1.00,0.95,0.85}, 1.15});
    sc.lights.push_back(Light{{-4, 6,-2},   {0.40,0.60,1.00}, 0.72});
    sc.lights.push_back(Light{{0, 12, 0},   {1.00,1.00,1.00}, 0.45});

    return sc;
}

// ═══════════════════════════════════════════════════════════════
//  SIGNAL HANDLER  (restore terminal on Ctrl+C)
// ═══════════════════════════════════════════════════════════════

static void onExit(int) {
    std::cout << "\033[?25h\033[0m\n\n"
              << "  \033[32mRay Tracer terminated.\033[0m\n\n";
    std::exit(0);
}

// ═══════════════════════════════════════════════════════════════
//  INTRO BANNER
// ═══════════════════════════════════════════════════════════════

void showBanner() {
    const char* colors[] = {
        "\033[31m", "\033[33m", "\033[32m",
        "\033[36m", "\033[34m", "\033[35m"
    };

    std::string title =
        "  ██████╗  █████╗ ██╗   ██╗    ████████╗██████╗  █████╗  ██████╗███████╗██████╗ \n"
        "  ██╔══██╗██╔══██╗╚██╗ ██╔╝       ██╔══╝██╔══██╗██╔══██╗██╔════╝██╔════╝██╔══██╗\n"
        "  ██████╔╝███████║ ╚████╔╝        ██║   ██████╔╝███████║██║     █████╗  ██████╔╝\n"
        "  ██╔══██╗██╔══██║  ╚██╔╝         ██║   ██╔══██╗██╔══██║██║     ██╔══╝  ██╔══██╗\n"
        "  ██║  ██║██║  ██║   ██║          ██║   ██║  ██║██║  ██║╚██████╗███████╗██║  ██║\n"
        "  ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝          ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚══════╝╚═╝  ╚═╝\n";

    for (int flash = 0; flash < 4; ++flash) {
        std::cout << "\033[2J\033[H" << colors[flash % 6] << title << "\033[0m";
        std::cout << "\n  \033[1;37mC++ Terminal Ray Tracer — ASCII Art Edition\033[0m\n";
        std::cout << "  \033[90mPhong shading · Reflections · Shadows · 2×2 Anti-Aliasing · Animation\033[0m\n\n";
        std::cout << "  \033[33mInitializing scene...\033[0m\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(180));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
}

// ═══════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════

int main(int argc, char* argv[]) {
    std::signal(SIGINT, onExit);

    // Optional: ./raytracer W H
    int W = (argc >= 3) ? std::atoi(argv[1]) : 88;
    int H = (argc >= 3) ? std::atoi(argv[2]) : 32;

    showBanner();
    std::cout << "\033[2J\033[?25l";  // clear + hide cursor

    // Camera: positioned above and behind, looking at origin
    Camera cam{
        Vec3{0, 3.8, 9.5},   // eye position
        Vec3{0, 0.6, 0},     // look at
        Vec3{0, 1, 0},       // world up
        52.0,                // vertical FOV (degrees)
        (double)W / H / 2.05 // aspect ratio (÷2 because chars are ~2× taller)
    };

    Renderer renderer(W, H, /*max reflections=*/4);

    int    frame     = 0;
    double totalTime = 0.0;
    double bestFPS   = 0.0;

    while (true) {
        double animTime = frame * 0.045;
        Scene  sc       = buildScene(animTime);

        auto t0 = std::chrono::steady_clock::now();
        renderer.render(sc, cam);
        auto t1 = std::chrono::steady_clock::now();

        double ms  = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double fps = 1000.0 / ms;
        totalTime += ms / 1000.0;
        bestFPS    = std::max(bestFPS, fps);

        // Status bar
        std::cout << "\033[0m\033[33m"
                  << "  Frame: " << std::setw(5) << frame + 1
                  << "  |  " << std::setw(5) << (int)ms << "ms"
                  << "  |  FPS: "  << std::setw(3) << (int)fps
                  << "  |  Best: " << std::setw(3) << (int)bestFPS
                  << "  |  Objects: " << std::setw(2) << sc.spheres.size()
                  << "  |  Lights: " << sc.lights.size()
                  << "  |  Depth: 4"
                  << "  |  \033[90mCtrl+C = exit\033[33m"
                  << "\033[0m   \n";

        ++frame;

        // Cap to ~12 FPS to avoid burning CPU needlessly
        int target_ms = 83;
        int sleep_ms  = std::max(0, target_ms - (int)ms);
        if (sleep_ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
}
