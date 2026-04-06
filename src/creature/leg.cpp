#include "platform.h"
#include "leg.h"
#include <algorithm>

void Leg::init(float ax, float az, float outAngle, bool left, int legIndex)
{
    attachX = ax;
    attachZ = az;
    outwardAngle = outAngle;
    leftSide = left;
    m_legIndex = legIndex;

    // Tripod gait: legs 0,3,4 in phase; legs 1,2,5 opposite
    // (L-front, R-mid, L-rear) vs (R-front, L-mid, R-rear)
    bool groupA = (legIndex == 0 || legIndex == 3 || legIndex == 4);
    m_phase = groupA ? 0.0f : (float)M_PI;

    // Initial pose: legs spread outward and down
    coxa.angle = 0;
    femur.angle = 0.3f;    // Slightly forward/down
    tibia.angle = -0.7f;   // Angled down toward ground
}

void Leg::update(float dt, float motorSignal)
{
    float speed = (std::max)(0.01f, std::abs(motorSignal));
    float stepFreq = 3.0f;

    m_phase += stepFreq * speed * dt * 2.0f * (float)M_PI;
    if (m_phase > 2.0f * (float)M_PI) m_phase -= 2.0f * (float)M_PI;

    float swing = sinf(m_phase);         // -1..1 forward/backward
    float liftPhase = cosf(m_phase);
    float lift = (std::max)(0.0f, liftPhase);  // 0..1 lift during swing

    float dir = (motorSignal >= 0) ? 1.0f : -1.0f;

    // Coxa: swing forward/backward (horizontal rotation)
    coxa.targetAngle = swing * 0.35f * dir;
    coxa.angle += (coxa.targetAngle - coxa.angle) * 12.0f * dt;
    coxa.angle = (std::clamp)(coxa.angle, -0.5f, 0.5f);

    // Femur: slight lift during swing phase
    femur.targetAngle = 0.3f + lift * 0.35f;
    femur.angle += (femur.targetAngle - femur.angle) * 10.0f * dt;
    femur.angle = (std::clamp)(femur.angle, 0.0f, 0.9f);

    // Tibia: fold more when lifted, extend when on ground
    tibia.targetAngle = -0.7f - lift * 0.25f + (1.0f - lift) * 0.1f;
    tibia.angle += (tibia.targetAngle - tibia.angle) * 10.0f * dt;
    tibia.angle = (std::clamp)(tibia.angle, -1.3f, 0.0f);

    // Check ground contact
    float hx, hy, hz, kx, ky, kz, fx, fy, fz;
    getJointPositions(hx, hy, hz, kx, ky, kz, fx, fy, fz);
    m_onGround = (fy <= 0.01f);
    m_groundForce = m_onGround ? (std::max)(0.0f, (0.01f - fy) * 80.0f) : 0.0f;
}

void Leg::applyGroundContact(float groundY)
{
    float hx, hy, hz, kx, ky, kz, fx, fy, fz;
    getJointPositions(hx, hy, hz, kx, ky, kz, fx, fy, fz);
    if (fy < groundY) {
        // Pull tibia up slightly to prevent ground penetration
        tibia.angle += (groundY - fy) * 1.5f;
        tibia.angle = (std::clamp)(tibia.angle, -1.3f, 0.0f);
        m_onGround = true;
        m_groundForce = (groundY - fy) * 100.0f;
    }
}

void Leg::getJointPositions(float& hipX, float& hipY, float& hipZ,
                             float& kneeX, float& kneeY, float& kneeZ,
                             float& footX, float& footY, float& footZ) const
{
    // Outward direction in XZ plane
    float outX = cosf(outwardAngle);
    float outZ = sinf(outwardAngle);

    // Perpendicular direction (for swing)
    float swingX = -sinf(outwardAngle);
    float swingZ = cosf(outwardAngle);

    // Hip/coxa: swings forward/backward, extends outward
    hipX = attachX + outX * coxa.length + swingX * coxa.angle * coxa.length;
    hipY = 0.0f;  // Hip is at body height (0 in local body space)
    hipZ = attachZ + outZ * coxa.length + swingZ * coxa.angle * coxa.length;

    // Femur: extends outward and downward from hip
    // femur.angle: 0=horizontal outward, positive=more up
    float femurOutLen = femur.length * cosf(femur.angle);
    float femurDownLen = -femur.length * 0.35f - femur.length * sinf(femur.angle) * 0.4f;

    kneeX = hipX + outX * femurOutLen;
    kneeY = hipY + femurDownLen;
    kneeZ = hipZ + outZ * femurOutLen;

    // Tibia: extends from knee outward and down to ground
    // tibia.angle: negative=down
    float tibiaOutLen = tibia.length * cosf(tibia.angle) * 0.5f;
    float tibiaDownLen = tibia.length * sinf(tibia.angle);

    footX = kneeX + outX * tibiaOutLen;
    footY = kneeY + tibiaDownLen;
    footZ = kneeZ + outZ * tibiaOutLen;
}
