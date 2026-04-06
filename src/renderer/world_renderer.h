#pragma once
#include "platform.h"
#include "world/world.h"
#include <QOpenGLFunctions_3_3_Core>

class WorldRenderer {
public:
    void draw(QOpenGLFunctions_3_3_Core* gl, const World& world);

private:
    void drawGround(QOpenGLFunctions_3_3_Core* gl, float size);
    void drawFood(QOpenGLFunctions_3_3_Core* gl, const Food& food);
    void drawObstacle(QOpenGLFunctions_3_3_Core* gl, const Obstacle& obs);
    void drawBoundary(QOpenGLFunctions_3_3_Core* gl, float size);
    void drawSphere(float x, float y, float z, float r, int sl = 12, int st = 8);
};
