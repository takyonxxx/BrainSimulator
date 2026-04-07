#include "platform.h"
#include "leg.h"
#include <algorithm>

void Leg::init(float ax, float az, float outDirX, float outDirZ,
               bool left, int legIndex)
{
    attachX = ax;
    attachZ = az;
    leftSide = left;
    m_legIndex = legIndex;

    // Normalize outward direction
    float len = sqrtf(outDirX * outDirX + outDirZ * outDirZ);
    m_outX = outDirX / len;
    m_outZ = outDirZ / len;

    // Forward direction = perpendicular to outward (rotated 90 CCW in XZ)
    m_fwdX = -m_outZ;
    m_fwdZ =  m_outX;

    // Tripod gait: 0,3,4 group A; 1,2,5 group B
    bool groupA = (legIndex == 0 || legIndex == 3 || legIndex == 4);
    m_phase = groupA ? 0.0f : (float)M_PI;

    // Rest pose
    swingAngle = 0;
    liftAngle = 0;
    kneeAngle = 0;
}

void Leg::update(float dt, float motorSignal)
{
    float speed = (std::max)(0.01f, std::abs(motorSignal));
    float dir = (motorSignal >= 0) ? 1.0f : -1.0f;

    // Advance gait phase
    m_phase += 3.0f * speed * dt * 2.0f * (float)M_PI;
    if (m_phase > 2.0f * (float)M_PI) m_phase -= 2.0f * (float)M_PI;

    float swingCycle = sinf(m_phase);
    float liftCycle = (std::max)(0.0f, cosf(m_phase));  // Lift during swing forward

    // Coxa swing: forward/backward
    float targetSwing = swingCycle * 0.4f * dir;
    swingAngle += (targetSwing - swingAngle) * 12.0f * dt;
    swingAngle = (std::clamp)(swingAngle, -0.5f, 0.5f);

    // Femur lift: raise leg during swing phase
    float targetLift = liftCycle * 0.4f;  // 0 to 0.4 rad lift
    liftAngle += (targetLift - liftAngle) * 10.0f * dt;
    liftAngle = (std::clamp)(liftAngle, -0.1f, 0.5f);

    // Knee bend: fold slightly when lifted
    float targetKnee = liftCycle * 0.3f;
    kneeAngle += (targetKnee - kneeAngle) * 10.0f * dt;
    kneeAngle = (std::clamp)(kneeAngle, -0.1f, 0.5f);

    // Ground contact check
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
        m_onGround = true;
        m_groundForce = (groundY - fy) * 100.0f;
    }
}

void Leg::getJointPositions(float& hipX, float& hipY, float& hipZ,
                             float& kneeX, float& kneeY, float& kneeZ,
                             float& footX, float& footY, float& footZ) const
{
    // === HIP (end of coxa) ===
    // Coxa extends outward from body + swings forward/backward
    hipX = attachX + m_outX * coxaLen + m_fwdX * swingAngle * coxaLen;
    hipY = 0.0f;
    hipZ = attachZ + m_outZ * coxaLen + m_fwdZ * swingAngle * coxaLen;

    // === KNEE (end of femur) ===
    // Femur goes outward and DOWN from hip
    // liftAngle raises the knee (less downward drop)
    float femurDrop = -femurLen * (0.5f - liftAngle * 0.6f);  // Y component (negative = down)
    float femurOut  =  femurLen * 0.85f;                       // Outward reach

    kneeX = hipX + m_outX * femurOut;
    kneeY = hipY + femurDrop;
    kneeZ = hipZ + m_outZ * femurOut;

    // === FOOT (end of tibia) ===
    // Tibia goes from knee mostly DOWN to ground, slightly outward
    float tibiaDrop = -tibiaLen * (0.75f - kneeAngle * 0.5f);  // Mostly down
    float tibiaOut  =  tibiaLen * 0.35f;                        // Slight outward

    footX = kneeX + m_outX * tibiaOut;
    footY = kneeY + tibiaDrop;
    footZ = kneeZ + m_outZ * tibiaOut;
}
