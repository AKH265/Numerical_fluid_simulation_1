#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <GLFW/glfw3.h>
#include "fluid_struct.h"

void renderD(FluidSquare &square) {
    const float cellSize = 2.0f / static_cast<float>(square.size);

    glBegin(GL_QUADS);
    for (int i = 0; i < square.size; i++) {
        for (int j = 0; j < square.size; j++) {
            float d = square.density[IX(i, j)];
            float intensity = std::min(d / 255.0f, 1.0f);

            glColor3f(intensity, intensity, intensity);

            float x0 = -1.0f + static_cast<float>(i) * cellSize;
            float y0 = -1.0f + static_cast<float>(j) * cellSize;
            float x1 = x0 + cellSize;
            float y1 = y0 + cellSize;

            glVertex2f(x0, y0);
            glVertex2f(x1, y0);
            glVertex2f(x1, y1);
            glVertex2f(x0, y1);
        }
    }
    glEnd();
}

void renderV(FluidSquare &square) {
    glBegin(GL_LINES);

    for (int i = 0; i < square.size; i++) {
        for (int j = 0; j < square.size; j++) {
            float vx = square.Vx[IX(i, j)];
            float vy = square.Vy[IX(i, j)];

            if (!(std::abs(vx) < 0.1f && std::abs(vy) <= 0.1f)) {
                glColor3f(1.0f, 1.0f, 1.0f);

                float x1 = static_cast<float>(i) * SCALE;
                float y1 = static_cast<float>(j) * SCALE;
                float x2 = x1 + vx * SCALE;
                float y2 = y1 + vy * SCALE;

                glVertex2f(2.0f * x1 / static_cast<float>(square.size * SCALE) - 1.0f,
                           2.0f * y1 / static_cast<float>(square.size * SCALE) - 1.0f);
                glVertex2f(2.0f * x2 / static_cast<float>(square.size * SCALE) - 1.0f,
                           2.0f * y2 / static_cast<float>(square.size * SCALE) - 1.0f);
            }
        }
    }

    glEnd();
}

void fadeD(FluidSquare &square) {
    const int total = square.size * square.size;
    for (int i = 0; i < total; i++) {
        square.density[i] = std::max(0.0f, square.density[i] - 0.02f);
    }
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Fluid Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    FluidSquare fluid = FluidSquareCreate(0.2f, 0.0f, 0.0001f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        int cx = fluid.size / 2;
        int cy = fluid.size / 2;

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                float amount = 50.0f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 100.0f;
                addDensity(fluid, cx + i, cy + j, amount);
            }
        }

        for (int i = 0; i < 2; i++) {
            float angle = noise(g_noiseTime) * 2.0f * 3.1415926f * 2.0f;
            float vx = std::cos(angle) * 0.2f;
            float vy = std::sin(angle) * 0.2f;
            g_noiseTime += 0.01f;

            addVelocity(fluid, cx, cy, vx, vy);
        }

        step(fluid);
        renderD(fluid);
        renderV(fluid);
        fadeD(fluid);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
