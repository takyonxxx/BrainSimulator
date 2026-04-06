#include "platform.h"
#include "creature_renderer.h"
#include <cmath>


void CreatureRenderer::draw(QOpenGLFunctions_3_3_Core* gl, const Creature& creature)
{
    glPushMatrix();
    glTranslatef(creature.x(), creature.y(), creature.z());
    glRotatef(creature.angle() * 180.0f / M_PI, 0, 1, 0);

    drawBody(gl, creature);
    drawLegs(gl, creature);
    drawAntennae(gl, creature);
    drawEyes(gl, creature);

    glPopMatrix();
}

void CreatureRenderer::drawBody(QOpenGLFunctions_3_3_Core* gl, const Creature& c)
{
    // Color based on behavior
    switch (c.currentBehavior()) {
    case Creature::Behavior::Exploring:
        glColor3f(0.2f, 0.6f, 0.3f); break;  // Green
    case Creature::Behavior::Feeding:
        glColor3f(0.1f, 0.4f, 0.8f); break;  // Blue
    case Creature::Behavior::Avoiding:
        glColor3f(0.8f, 0.2f, 0.2f); break;  // Red
    case Creature::Behavior::Turning:
        glColor3f(0.7f, 0.5f, 0.1f); break;  // Orange
    case Creature::Behavior::Idle:
        glColor3f(0.4f, 0.4f, 0.4f); break;  // Gray
    }

    // Main body - thorax (ellipsoid)
    drawEllipsoid(gl, 0, 0, 0,
                  Creature::BODY_WIDTH * 0.5f,
                  Creature::BODY_HEIGHT * 0.5f,
                  Creature::BODY_LENGTH * 0.5f);

    // Head
    float headDark = 0.7f;
    glColor3f(0.15f * headDark, 0.45f * headDark, 0.25f * headDark);
    drawEllipsoid(gl, 0, 0.02f, Creature::BODY_LENGTH * 0.45f,
                  Creature::BODY_WIDTH * 0.35f,
                  Creature::BODY_HEIGHT * 0.4f,
                  0.12f);

    // Abdomen
    glColor3f(0.25f, 0.5f, 0.3f);
    drawEllipsoid(gl, 0, -0.01f, -Creature::BODY_LENGTH * 0.4f,
                  Creature::BODY_WIDTH * 0.4f,
                  Creature::BODY_HEIGHT * 0.45f,
                  0.15f);

    // Energy indicator - subtle glow on back
    float e = c.energy();
    glColor3f(e * 0.3f, e * 0.8f, e * 0.3f);
    drawEllipsoid(gl, 0, Creature::BODY_HEIGHT * 0.3f, 0,
                  0.04f, 0.02f, 0.06f);
}

void CreatureRenderer::drawLegs(QOpenGLFunctions_3_3_Core* gl, const Creature& c)
{
    float legRadius = 0.012f;

    for (int i = 0; i < 6; i++) {
        const auto& leg = c.legs()[i];

        // Get joint positions from leg kinematics
        float hx, hy, hz, kx, ky, kz, fx, fy, fz;
        leg.getJointPositions(hx, hy, hz, kx, ky, kz, fx, fy, fz);

        // Body attachment point
        float bx = leg.attachX;
        float bz = leg.attachZ;

        // Color based on ground contact
        if (leg.isOnGround()) {
            glColor3f(0.35f, 0.30f, 0.20f);
        } else {
            glColor3f(0.25f, 0.20f, 0.12f);
        }

        // Draw segments: body->hip, hip->knee, knee->foot
        drawCylinder(gl, bx, 0.0f, bz, hx, hy, hz, legRadius * 1.3f);
        drawCylinder(gl, hx, hy, hz, kx, ky, kz, legRadius);
        drawCylinder(gl, kx, ky, kz, fx, fy, fz, legRadius * 0.8f);

        // Joint spheres
        drawSphere(gl, hx, hy, hz, legRadius * 1.5f, 6, 4);
        drawSphere(gl, kx, ky, kz, legRadius * 1.2f, 6, 4);

        // Foot
        if (leg.isOnGround()) {
            glColor3f(0.4f, 0.35f, 0.2f);
        }
        drawSphere(gl, fx, fy, fz, legRadius * 1.0f, 6, 4);
    }
}

void CreatureRenderer::drawAntennae(QOpenGLFunctions_3_3_Core* gl, const Creature& c)
{
    float headZ = Creature::BODY_LENGTH * 0.45f;
    float antLen = 0.2f;

    // Left antenna
    float chemL = c.sensors().chemLeft();
    glColor3f(0.4f + chemL * 0.5f, 0.3f, 0.1f);
    float lx1 = -0.05f, lz1 = headZ + 0.05f;
    float lx2 = -0.12f, lz2 = headZ + antLen;
    float ly2 = 0.08f + sinf(c.distanceTraveled() * 5.0f) * 0.02f;
    drawCylinder(gl, lx1, 0.04f, lz1, lx2, ly2, lz2, 0.005f);
    drawSphere(gl, lx2, ly2, lz2, 0.01f, 6, 4);

    // Right antenna
    float chemR = c.sensors().chemRight();
    glColor3f(0.4f + chemR * 0.5f, 0.3f, 0.1f);
    float rx1 = 0.05f, rz1 = headZ + 0.05f;
    float rx2 = 0.12f, rz2 = headZ + antLen;
    float ry2 = 0.08f + sinf(c.distanceTraveled() * 5.0f + 1.0f) * 0.02f;
    drawCylinder(gl, rx1, 0.04f, rz1, rx2, ry2, rz2, 0.005f);
    drawSphere(gl, rx2, ry2, rz2, 0.01f, 6, 4);
}

void CreatureRenderer::drawEyes(QOpenGLFunctions_3_3_Core* gl, const Creature& c)
{
    float headZ = Creature::BODY_LENGTH * 0.45f;

    // Eye whites
    glColor3f(0.9f, 0.9f, 0.85f);
    drawSphere(gl, -0.06f, 0.05f, headZ + 0.06f, 0.025f, 8, 6);
    drawSphere(gl,  0.06f, 0.05f, headZ + 0.06f, 0.025f, 8, 6);

    // Pupils - look toward food if detected
    float lookX = 0, lookZ = 0.01f;
    if (c.sensors().chemLeft() > 0.2f) lookX -= 0.005f;
    if (c.sensors().chemRight() > 0.2f) lookX += 0.005f;

    glColor3f(0.05f, 0.05f, 0.05f);
    drawSphere(gl, -0.06f + lookX, 0.055f, headZ + 0.08f + lookZ, 0.012f, 6, 4);
    drawSphere(gl,  0.06f + lookX, 0.055f, headZ + 0.08f + lookZ, 0.012f, 6, 4);
}

// ===== PRIMITIVE DRAWING =====

void CreatureRenderer::drawSphere(QOpenGLFunctions_3_3_Core*, float x, float y, float z,
                                  float radius, int slices, int stacks)
{
    for (int i = 0; i < stacks; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / stacks);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);
        float y0 = sinf(lat0), y1 = sinf(lat1);
        float r0 = cosf(lat0), r1 = cosf(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float lng = 2.0f * M_PI * (float)j / slices;
            float cx = cosf(lng), cz = sinf(lng);

            glNormal3f(cx * r0, y0, cz * r0);
            glVertex3f(x + radius * cx * r0, y + radius * y0, z + radius * cz * r0);
            glNormal3f(cx * r1, y1, cz * r1);
            glVertex3f(x + radius * cx * r1, y + radius * y1, z + radius * cz * r1);
        }
        glEnd();
    }
}

void CreatureRenderer::drawCylinder(QOpenGLFunctions_3_3_Core*,
                                    float x1, float y1, float z1,
                                    float x2, float y2, float z2,
                                    float radius, int segments)
{
    float dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len < 0.001f) return;

    // Build orthonormal basis
    float ax = dx / len, ay = dy / len, az = dz / len;
    float bx, by, bz;
    if (fabsf(ay) < 0.9f) {
        bx = 0; by = 1; bz = 0;
    } else {
        bx = 1; by = 0; bz = 0;
    }
    // Cross product a x b
    float cx = ay * bz - az * by;
    float cy = az * bx - ax * bz;
    float cz_ = ax * by - ay * bx;
    float cl = sqrtf(cx*cx + cy*cy + cz_*cz_);
    cx /= cl; cy /= cl; cz_ /= cl;
    // Second perpendicular
    bx = cy * az - cz_ * ay;
    by = cz_ * ax - cx * az;
    bz = cx * ay - cy * ax;

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float cosA = cosf(angle), sinA = sinf(angle);
        float nx = cx * cosA + bx * sinA;
        float ny = cy * cosA + by * sinA;
        float nz = cz_ * cosA + bz * sinA;
        glNormal3f(nx, ny, nz);
        glVertex3f(x1 + radius * nx, y1 + radius * ny, z1 + radius * nz);
        glVertex3f(x2 + radius * nx, y2 + radius * ny, z2 + radius * nz);
    }
    glEnd();
}

void CreatureRenderer::drawEllipsoid(QOpenGLFunctions_3_3_Core*,
                                     float cx, float cy, float cz,
                                     float rx, float ry, float rz,
                                     int slices, int stacks)
{
    for (int i = 0; i < stacks; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / stacks);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float lng = 2.0f * M_PI * (float)j / slices;
            for (int k = 0; k < 2; k++) {
                float lat = (k == 0) ? lat0 : lat1;
                float x = cosf(lat) * cosf(lng);
                float y = sinf(lat);
                float z = cosf(lat) * sinf(lng);
                // Normal (approximate for ellipsoid)
                glNormal3f(x / rx, y / ry, z / rz);
                glVertex3f(cx + rx * x, cy + ry * y, cz + rz * z);
            }
        }
        glEnd();
    }
}
