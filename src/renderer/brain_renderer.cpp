#include "platform.h"
#include "brain_renderer.h"
#include <cmath>


void BrainRenderer::draw(QOpenGLFunctions_3_3_Core* gl, const Brain& brain,
                         float cx, float cy, float cz, float scale)
{
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glScalef(scale, scale, scale);

    // Draw active synapses first (behind neurons)
    drawSynapseLines(gl, brain, 1.0f);

    // Draw neurons
    for (auto& neuron : brain.neurons()) {
        float x = neuron.posX;
        float y = neuron.posY;
        float z = neuron.posZ;
        float activity = neuron.activity();
        float radius = 0.02f + activity * 0.03f;

        drawNeuron(x, y, z, activity, neuron.type(), neuron.hasFired(), radius);
    }

    glPopMatrix();
}

void BrainRenderer::drawNeuron(float x, float y, float z, float activity,
                                NeuronType type, bool fired, float radius)
{
    // Color by type
    float r = 0.3f, g = 0.3f, b = 0.3f;
    switch (type) {
    case NeuronType::Sensory:
        r = 0.2f + activity * 0.8f;
        g = 0.6f + activity * 0.4f;
        b = 0.1f;
        break;
    case NeuronType::Inter:
        r = 0.1f;
        g = 0.3f + activity * 0.5f;
        b = 0.5f + activity * 0.5f;
        break;
    case NeuronType::Motor:
        r = 0.7f + activity * 0.3f;
        g = 0.2f + activity * 0.3f;
        b = 0.1f;
        break;
    case NeuronType::Modulatory:
        r = 0.6f + activity * 0.4f;
        g = 0.1f;
        b = 0.6f + activity * 0.4f;
        break;
    }

    if (fired) {
        r = (std::min)(1.0f, r + 0.5f);
        g = (std::min)(1.0f, g + 0.5f);
        b = (std::min)(1.0f, b + 0.5f);
        radius *= 1.5f;
    }

    glColor3f(r, g, b);

    // Simple sphere
    int sl = 6, st = 4;
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
                glVertex3f(x + radius * nx, y + radius * ny, z + radius * nz);
            }
        }
        glEnd();
    }

    // Firing glow
    if (fired) {
        glColor4f(1, 1, 0.5f, 0.3f);
        float gr = radius * 2.0f;
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(x, y, z);
        for (int i = 0; i <= 12; i++) {
            float a = 2.0f * M_PI * i / 12.0f;
            glVertex3f(x + gr * cosf(a), y + gr * sinf(a), z);
        }
        glEnd();
    }
}

void BrainRenderer::drawSynapseLines(QOpenGLFunctions_3_3_Core* gl,
                                      const Brain& brain, float scale)
{
    glLineWidth(1.0f);
    glBegin(GL_LINES);

    for (auto& syn : brain.synapses()) {
        auto& pre = brain.neurons()[syn.preNeuronId];
        auto& post = brain.neurons()[syn.postNeuronId];

        float activity = pre.activity();
        if (activity < 0.05f) continue;  // Only show active synapses

        float alpha = activity * 0.6f;
        if (syn.weight > 0) {
            glColor4f(0.2f, 0.5f + activity * 0.5f, 0.8f, alpha);
        } else {
            glColor4f(0.8f, 0.2f, 0.2f + activity * 0.3f, alpha);
        }

        glVertex3f(pre.posX, pre.posY, pre.posZ);
        glVertex3f(post.posX, post.posY, post.posZ);
    }
    glEnd();
}

void BrainRenderer::drawHUD(const Brain& brain, int windowW, int windowH)
{
    // Switch to 2D overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowW, windowH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Neural activity bar graph (bottom-right)
    int barX = windowW - 220;
    int barY = windowH - 120;
    int barW = 200;
    int barH = 100;

    // Background
    glColor4f(0, 0, 0, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(barX - 5, barY - 5);
    glVertex2f(barX + barW + 5, barY - 5);
    glVertex2f(barX + barW + 5, barY + barH + 5);
    glVertex2f(barX - 5, barY + barH + 5);
    glEnd();

    // Activity bars by neuron type
    float sensoryAct = 0, interAct = 0, motorAct = 0, modAct = 0;
    int sC = 0, iC = 0, mC = 0, modC = 0;
    for (auto& n : brain.neurons()) {
        switch (n.type()) {
        case NeuronType::Sensory: sensoryAct += n.activity(); sC++; break;
        case NeuronType::Inter: interAct += n.activity(); iC++; break;
        case NeuronType::Motor: motorAct += n.activity(); mC++; break;
        case NeuronType::Modulatory: modAct += n.activity(); modC++; break;
        }
    }
    if (sC > 0) sensoryAct /= sC;
    if (iC > 0) interAct /= iC;
    if (mC > 0) motorAct /= mC;
    if (modC > 0) modAct /= modC;

    float acts[] = {sensoryAct, interAct, motorAct, modAct};
    float colors[][3] = {{0.2f,0.8f,0.2f}, {0.2f,0.4f,0.9f}, {0.9f,0.4f,0.1f}, {0.7f,0.1f,0.7f}};

    int bw = barW / 4 - 4;
    for (int i = 0; i < 4; i++) {
        float h = acts[i] * barH;
        int bx = barX + i * (bw + 4);
        glColor3f(colors[i][0], colors[i][1], colors[i][2]);
        glBegin(GL_QUADS);
        glVertex2f(bx, barY + barH);
        glVertex2f(bx + bw, barY + barH);
        glVertex2f(bx + bw, barY + barH - h);
        glVertex2f(bx, barY + barH - h);
        glEnd();
    }

    // Neuromodulator levels (small bars)
    int modX = barX;
    int modY = barY - 25;
    float mods[] = {brain.dopamineLevel(), brain.serotoninLevel(), brain.octopamineLevel()};
    float modColors[][3] = {{0.1f,0.7f,0.9f}, {0.9f,0.7f,0.1f}, {0.9f,0.3f,0.3f}};
    for (int i = 0; i < 3; i++) {
        int mx = modX + i * 70;
        glColor3f(modColors[i][0] * 0.3f, modColors[i][1] * 0.3f, modColors[i][2] * 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(mx, modY); glVertex2f(mx + 60, modY);
        glVertex2f(mx + 60, modY + 10); glVertex2f(mx, modY + 10);
        glEnd();
        glColor3f(modColors[i][0], modColors[i][1], modColors[i][2]);
        glBegin(GL_QUADS);
        glVertex2f(mx, modY); glVertex2f(mx + 60 * mods[i], modY);
        glVertex2f(mx + 60 * mods[i], modY + 10); glVertex2f(mx, modY + 10);
        glEnd();
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
