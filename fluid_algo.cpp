#include <cmath>
#include "fluid_struct.h"

float g_noiseTime = 0.0f;

FluidSquare FluidSquareCreate(float dt, float diffusion, float viscosity) {
    FluidSquare square;
    square.size = N;
    square.dt = dt;
    square.diff = diffusion;
    square.visc = viscosity;

    square.s = std::vector<float>(N * N, 0.0f);
    square.density = std::vector<float>(N * N, 0.0f);
    square.Vx = std::vector<float>(N * N, 0.0f);
    square.Vy = std::vector<float>(N * N, 0.0f);
    square.Vx0 = std::vector<float>(N * N, 0.0f);
    square.Vy0 = std::vector<float>(N * N, 0.0f);

    return square;
}

float fade(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float hash1(int x) {
    x = (x << 13) ^ x;
    return 1.0f - static_cast<float>(
        ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff)
    ) / 1073741824.0f;
}

float noise(float x) {
    int x0 = static_cast<int>(std::floor(x));
    int x1 = x0 + 1;

    float t = x - static_cast<float>(x0);
    float g0 = hash1(x0);
    float g1 = hash1(x1);

    float n0 = g0 * (x - static_cast<float>(x0));
    float n1 = g1 * (x - static_cast<float>(x1));

    return 0.5f * (lerp(n0, n1, fade(t)) + 1.0f);
}

void addDensity(FluidSquare &square, int x, int y, float amount) {
    if (x < 0 || x >= square.size || y < 0 || y >= square.size) {
        return;
    }
    square.density[IX(x, y)] += amount;
}

void addVelocity(FluidSquare &square, int x, int y, float amountX, float amountY) {
    if (x < 0 || x >= square.size || y < 0 || y >= square.size) {
        return;
    }
    square.Vx[IX(x, y)] += amountX;
    square.Vy[IX(x, y)] += amountY;
}

static void set_bnd(int b, std::vector<float> &x) {
    for (int i = 1; i < N - 1; i++) {
        x[IX(i, 0)]     = (b == 2) ? -x[IX(i, 1)]     : x[IX(i, 1)];
        x[IX(i, N - 1)] = (b == 2) ? -x[IX(i, N - 2)] : x[IX(i, N - 2)];
    }

    for (int j = 1; j < N - 1; j++) {
        x[IX(0, j)]     = (b == 1) ? -x[IX(1, j)]     : x[IX(1, j)];
        x[IX(N - 1, j)] = (b == 1) ? -x[IX(N - 2, j)] : x[IX(N - 2, j)];
    }

    x[IX(0, 0)]         = 0.5f * (x[IX(1, 0)]     + x[IX(0, 1)]);
    x[IX(0, N - 1)]     = 0.5f * (x[IX(1, N - 1)] + x[IX(0, N - 2)]);
    x[IX(N - 1, 0)]     = 0.5f * (x[IX(N - 2, 0)] + x[IX(N - 1, 1)]);
    x[IX(N - 1, N - 1)] = 0.5f * (x[IX(N - 2, N - 1)] + x[IX(N - 1, N - 2)]);
}

static void lin_solve(int b, std::vector<float> &x, const std::vector<float> &x0, float a, float c) {
    const float cRecip = 1.0f / c;

    for (int k = 0; k < iter; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                x[IX(i, j)] = (
                    x0[IX(i, j)] +
                    a * (
                        x[IX(i + 1, j)] +
                        x[IX(i - 1, j)] +
                        x[IX(i, j + 1)] +
                        x[IX(i, j - 1)]
                    )
                ) * cRecip;
            }
        }
        set_bnd(b, x);
    }
}

static void diffuse(int b, std::vector<float> &x, const std::vector<float> &x0, float diff, float dt) {
    const float a = dt * diff * static_cast<float>((N - 2) * (N - 2));
    lin_solve(b, x, x0, a, 1.0f + 4.0f * a);
}

static void project(std::vector<float> &velocX, std::vector<float> &velocY,
                    std::vector<float> &p, std::vector<float> &div) {
    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            div[IX(i, j)] = -0.5f * (
                velocX[IX(i + 1, j)] - velocX[IX(i - 1, j)] +
                velocY[IX(i, j + 1)] - velocY[IX(i, j - 1)]
            ) / static_cast<float>(N);
            p[IX(i, j)] = 0.0f;
        }
    }

    set_bnd(0, div);
    set_bnd(0, p);
    lin_solve(0, p, div, 1.0f, 4.0f);

    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)] - p[IX(i - 1, j)]) * static_cast<float>(N);
            velocY[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)] - p[IX(i, j - 1)]) * static_cast<float>(N);
        }
    }

    set_bnd(1, velocX);
    set_bnd(2, velocY);
}

static void advect(int b, std::vector<float> &d, const std::vector<float> &d0,
                   const std::vector<float> &velocX, const std::vector<float> &velocY,
                   float dt) {
    const float dtx = dt * static_cast<float>(N - 2);
    const float dty = dt * static_cast<float>(N - 2);
    const float Nfloat = static_cast<float>(N - 2);

    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            float x = static_cast<float>(i) - dtx * velocX[IX(i, j)];
            float y = static_cast<float>(j) - dty * velocY[IX(i, j)];

            if (x < 0.5f) x = 0.5f;
            if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
            int i0 = static_cast<int>(std::floor(x));
            int i1 = i0 + 1;

            if (y < 0.5f) y = 0.5f;
            if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
            int j0 = static_cast<int>(std::floor(y));
            int j1 = j0 + 1;

            const float s1 = x - static_cast<float>(i0);
            const float s0 = 1.0f - s1;
            const float t1 = y - static_cast<float>(j0);
            const float t0 = 1.0f - t1;

            d[IX(i, j)] =
                s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
                s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
        }
    }

    set_bnd(b, d);
}

void step(FluidSquare &square) {
    const float visc = square.visc;
    const float diff = square.diff;
    const float dt = square.dt;

    diffuse(1, square.Vx0, square.Vx, visc, dt);
    diffuse(2, square.Vy0, square.Vy, visc, dt);

    project(square.Vx0, square.Vy0, square.Vx, square.Vy);

    advect(1, square.Vx, square.Vx0, square.Vx0, square.Vy0, dt);
    advect(2, square.Vy, square.Vy0, square.Vx0, square.Vy0, dt);

    project(square.Vx, square.Vy, square.Vx0, square.Vy0);

    diffuse(0, square.s, square.density, diff, dt);
    advect(0, square.density, square.s, square.Vx, square.Vy, dt);
}
