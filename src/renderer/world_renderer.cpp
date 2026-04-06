#include "platform.h"
#include "world_renderer.h"
#include <cmath>


void WorldRenderer::draw(QOpenGLFunctions_3_3_Core* gl, const World& world)
{
    drawGround(gl, world.size());
    drawBoundary(gl, world.size());

    for (auto& food : world.foods()) {
        if (food.active) drawFood(gl, food);
    }
    for (auto& obs : world.obstacles()) {
        drawObstacle(gl, obs);
    }
}

void WorldRenderer::drawGround(QOpenGLFunctions_3_3_Core* gl, float size)
{
    int gridSize = 40;
    float step = size * 2.0f / gridSize;

    glBegin(GL_QUADS);
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            float x = -size + i * step;
            float z = -size + j * step;

            // Checkerboard with subtle variation
            bool dark = ((i + j) % 2 == 0);
            if (dark) {
                glColor3f(0.28f, 0.35f, 0.22f);  // Dark grass
            } else {
                glColor3f(0.32f, 0.40f, 0.25f);  // Light grass
            }

            glNormal3f(0, 1, 0);
            glVertex3f(x, 0, z);
            glVertex3f(x + step, 0, z);
            glVertex3f(x + step, 0, z + step);
            glVertex3f(x, 0, z + step);
        }
    }
    glEnd();
}

void WorldRenderer::drawFood(QOpenGLFunctions_3_3_Core* gl, const Food& food)
{
    // Glowing green sphere for food
    float pulse = 0.8f + 0.2f * sinf(food.x * 3.0f + food.z * 2.0f);
    float a = food.amount;
    glColor3f(0.1f * a, 0.8f * a * pulse, 0.1f * a);
    drawSphere(food.x, 0.08f + 0.03f * sinf(food.z * 2.0f), food.z,
               0.08f * a);

    // Scent field visualization (translucent ring)
    glColor4f(0.2f, 0.6f, 0.2f, 0.15f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 32; i++) {
        float angle = 2.0f * M_PI * i / 32.0f;
        glVertex3f(food.x + cosf(angle) * 1.5f, 0.01f,
                   food.z + sinf(angle) * 1.5f);
    }
    glEnd();

    // Inner ring
    glColor4f(0.3f, 0.8f, 0.3f, 0.25f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 24; i++) {
        float angle = 2.0f * M_PI * i / 24.0f;
        glVertex3f(food.x + cosf(angle) * 0.6f, 0.01f,
                   food.z + sinf(angle) * 0.6f);
    }
    glEnd();
}

void WorldRenderer::drawObstacle(QOpenGLFunctions_3_3_Core* gl, const Obstacle& obs)
{
    // Rock-like obstacle
    glColor3f(0.45f, 0.40f, 0.35f);
    drawSphere(obs.x, obs.radius * 0.4f, obs.z, obs.radius, 10, 8);

    // Shadow
    glColor4f(0, 0, 0, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(obs.x, 0.005f, obs.z);
    for (int i = 0; i <= 16; i++) {
        float angle = 2.0f * M_PI * i / 16.0f;
        glVertex3f(obs.x + cosf(angle) * obs.radius * 1.2f, 0.005f,
                   obs.z + sinf(angle) * obs.radius * 1.2f);
    }
    glEnd();
}

void WorldRenderer::drawBoundary(QOpenGLFunctions_3_3_Core* gl, float size)
{
    glColor3f(0.5f, 0.3f, 0.2f);
    float h = 0.3f;
    float w = 0.1f;

    // Four walls
    glBegin(GL_QUADS);
    // North
    glNormal3f(0, 0, -1);
    glVertex3f(-size, 0, size); glVertex3f(size, 0, size);
    glVertex3f(size, h, size); glVertex3f(-size, h, size);
    // South
    glNormal3f(0, 0, 1);
    glVertex3f(-size, 0, -size); glVertex3f(size, 0, -size);
    glVertex3f(size, h, -size); glVertex3f(-size, h, -size);
    // East
    glNormal3f(-1, 0, 0);
    glVertex3f(size, 0, -size); glVertex3f(size, 0, size);
    glVertex3f(size, h, size); glVertex3f(size, h, -size);
    // West
    glNormal3f(1, 0, 0);
    glVertex3f(-size, 0, -size); glVertex3f(-size, 0, size);
    glVertex3f(-size, h, size); glVertex3f(-size, h, -size);
    glEnd();
}

void WorldRenderer::drawSphere(float x, float y, float z, float r, int sl, int st)
{
    for (int i = 0; i < st; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / st);
        float lat1 = M_PI * (-0.5f + (float)(i+1) / st);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= sl; j++) {
            float lng = 2.0f * M_PI * j / sl;
            for (int k = 0; k < 2; k++) {
                float lat = (k == 0) ? lat0 : lat1;
                float nx = cosf(lat) * cosf(lng);
                float ny = sinf(lat);
                float nz = cosf(lat) * sinf(lng);
                glNormal3f(nx, ny, nz);
                glVertex3f(x + r * nx, y + r * ny, z + r * nz);
            }
        }
        glEnd();
    }
}
