#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <algorithm>

const int WIDTH = 1200;
const int HEIGHT = 800;

struct Color {
    uint8_t r, g, b;
};

// HSV to RGB
Color hsv_to_rgb(double h, double s, double v) {
    double c = v * s;
    double x = c * (1 - std::abs(std::fmod(h / 60.0, 2) - 1));
    double m = v - c;
    double r, g, b;
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    return {
        static_cast<uint8_t>((r + m) * 255),
        static_cast<uint8_t>((g + m) * 255),
        static_cast<uint8_t>((b + m) * 255)
    };
}

void render_row(int y, int width, int height, double center_x, double center_y, double scale,
                int max_iter, double hue_shift, double sat, double val,
                std::vector<uint32_t>& pixels, std::atomic<int>& progress) {
    double aspect = static_cast<double>(width) / height;
    double cy = center_y + (static_cast<double>(y) / height - 0.5) * scale;

    for (int x = 0; x < width; ++x) {
        double cx = center_x + (static_cast<double>(x) / width - 0.5) * scale * aspect;
        double zx = 0.0, zy = 0.0, zx2 = 0.0, zy2 = 0.0;
        int iter = 0;
        for (; iter < max_iter; ++iter) {
            if (zx2 + zy2 > 4.0) break;
            zy = 2.0 * zx * zy + cy;
            zx = zx2 - zy2 + cx;
            zx2 = zx * zx;
            zy2 = zy * zy;
        }

        uint32_t color = 0xFF000000; // black + alpha
        if (iter < max_iter) {
            double log_zn = std::log(zx2 + zy2) * 0.5;
            double nu = std::log(log_zn / std::log(2.0)) / std::log(2.0);
            double smooth = iter + 1 - nu;
            double t = smooth / max_iter;
            double h = std::fmod(t * 360.0 * 2.0 + hue_shift, 360.0);
            Color c = hsv_to_rgb(h, sat, val);
            color = (255 << 24) | (c.r << 16) | (c.g << 8) | c.b;
        }
        pixels[y * width + x] = color;
    }
    progress++;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Mandelbrot Explorer - C++",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    std::vector<uint32_t> pixels(WIDTH * HEIGHT);
    double center_x = -0.75, center_y = 0.0, scale = 2.5;
    int max_iter = 300;
    double hue_shift = 0.0, sat = 0.8, val = 1.0;
    bool need_redraw = true;
    bool dragging = false;
    int last_mx = 0, last_my = 0;

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                dragging = true;
                last_mx = e.button.x;
                last_my = e.button.y;
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
            }
            else if (e.type == SDL_MOUSEMOTION && dragging) {
                double dx = (e.motion.x - last_mx) / static_cast<double>(WIDTH);
                double dy = (e.motion.y - last_my) / static_cast<double>(HEIGHT);
                double aspect = static_cast<double>(WIDTH) / HEIGHT;
                center_x += dx * scale * aspect;  // fixed left/right direction
                center_y += dy * scale;
                last_mx = e.motion.x;
                last_my = e.motion.y;
                need_redraw = true;
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                double factor = (e.wheel.y > 0) ? 0.85 : 1.15;
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                double aspect = static_cast<double>(WIDTH) / HEIGHT;
                double off_x = (mx / static_cast<double>(WIDTH) - 0.5) * scale * aspect;
                double off_y = (0.5 - my / static_cast<double>(HEIGHT)) * scale;
                center_x += off_x * (1 - factor);
                center_y += off_y * (1 - factor);
                scale *= factor;
                need_redraw = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_r:
                        center_x = -0.75; center_y = 0.0; scale = 2.5;
                        need_redraw = true;
                        break;
                    case SDLK_UP: max_iter = std::max(50, max_iter - 50); need_redraw = true; break;
                    case SDLK_DOWN: max_iter = std::min(2000, max_iter + 50); need_redraw = true; break;
                    case SDLK_LEFT: hue_shift = std::fmod(hue_shift - 10 + 360, 360); need_redraw = true; break;
                    case SDLK_RIGHT: hue_shift = std::fmod(hue_shift + 10, 360); need_redraw = true; break;
                    case SDLK_PLUS: case SDLK_EQUALS: sat = std::min(1.0, sat + 0.05); need_redraw = true; break;
                    case SDLK_MINUS: sat = std::max(0.0, sat - 0.05); need_redraw = true; break;
                    case SDLK_SPACE: {
                        double factor = 0.85;
                        int mx, my;
                        SDL_GetMouseState(&mx, &my);
                        double aspect = static_cast<double>(WIDTH) / HEIGHT;
                        double off_x = (mx / static_cast<double>(WIDTH) - 0.5) * scale * aspect;
                        double off_y = (0.5 - my / static_cast<double>(HEIGHT)) * scale;
                        center_x += off_x * (1 - factor);
                        center_y += off_y * (1 - factor);
                        scale *= factor;
                        need_redraw = true;
                        break;
                    }
                }
            }
        }

        if (need_redraw) {
            unsigned int num_threads = std::thread::hardware_concurrency();
            if (num_threads == 0) num_threads = 4;
            std::vector<std::thread> threads;
            std::atomic<int> progress{0};

            int rows_per_thread = HEIGHT / num_threads;
            for (unsigned int t = 0; t < num_threads; ++t) {
                int start_y = t * rows_per_thread;
                int end_y = (t == num_threads - 1) ? HEIGHT : start_y + rows_per_thread;
                threads.emplace_back([=, &pixels, &progress]() {
                    for (int y = start_y; y < end_y; ++y) {
                        render_row(y, WIDTH, HEIGHT, center_x, center_y, scale,
                                   max_iter, hue_shift, sat, val, pixels, progress);
                    }
                });
            }
            for (auto& th : threads) th.join();

            SDL_UpdateTexture(texture, nullptr, pixels.data(), WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
            need_redraw = false;
        }

        SDL_Delay(10);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
