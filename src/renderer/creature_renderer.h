#pragma once
#include "platform.h"
#include "creature/creature.h"
#include <QOpenGLFunctions_3_3_Core>

class CreatureRenderer {
public:
    void draw(QOpenGLFunctions_3_3_Core* gl, const Creature& creature);

private:
    void drawBody(QOpenGLFunctions_3_3_Core* gl, const Creature& c);
    void drawLegs(QOpenGLFunctions_3_3_Core* gl, const Creature& c);
    void drawAntennae(QOpenGLFunctions_3_3_Core* gl, const Creature& c);
    void drawEyes(QOpenGLFunctions_3_3_Core* gl, const Creature& c);

    void drawSphere(QOpenGLFunctions_3_3_Core* gl, float x, float y, float z,
                    float radius, int slices = 12, int stacks = 8);
    void drawCylinder(QOpenGLFunctions_3_3_Core* gl,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      float radius, int segments = 8);
    void drawEllipsoid(QOpenGLFunctions_3_3_Core* gl,
                       float cx, float cy, float cz,
                       float rx, float ry, float rz,
                       int slices = 16, int stacks = 12);
};
