#pragma once
#include "platform.h"
#include "brain/brain.h"
#include <QOpenGLFunctions_3_3_Core>

class BrainRenderer {
public:
    void draw(QOpenGLFunctions_3_3_Core* gl, const Brain& brain,
              float cx, float cy, float cz, float scale = 0.5f);
    void drawHUD(const Brain& brain, int windowW, int windowH);

private:
    void drawNeuron(float x, float y, float z, float activity,
                    NeuronType type, bool fired, float radius);
    void drawSynapseLines(QOpenGLFunctions_3_3_Core* gl, const Brain& brain, float scale);
};
