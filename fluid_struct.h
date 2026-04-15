#ifndef FLUID_STRUCT_H
#define FLUID_STRUCT_H

#include <vector>

const int N = 256;
const int iter = 16;
const int SCALE = 4;
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

struct FluidSquare {
    int size;
    float dt;
    float diff;
    float visc;

    std::vector<float> s;
    std::vector<float> density;

    std::vector<float> Vx;
    std::vector<float> Vy;

    std::vector<float> Vx0;
    std::vector<float> Vy0;
};

extern float g_noiseTime;

inline int IX(int x, int y) {
    return x + y * N;
}

FluidSquare FluidSquareCreate(float dt, float diffusion, float viscosity);

float fade(float t);
float lerp(float a, float b, float t);
float hash1(int x);
float noise(float x);

void addDensity(FluidSquare &square, int x, int y, float amount);
void addVelocity(FluidSquare &square, int x, int y, float amountX, float amountY);
void step(FluidSquare &square);

#endif
