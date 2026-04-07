#pragma once
#include "platform.h"
#include <cmath>

// Insect leg - 3 segments (coxa, femur, tibia)
// Body local coords: X=right, Y=up, Z=forward

class Leg {
public:
    Leg() = default;

    void init(float attachX, float attachZ,
              float outDirX, float outDirZ,   // Unit vector: direction leg points outward
              bool left, int legIndex);

    void update(float dt, float motorSignal);
    void applyGroundContact(float groundY);

    // Returns joint world positions in body-local space
    void getJointPositions(float& hipX, float& hipY, float& hipZ,
                           float& kneeX, float& kneeY, float& kneeZ,
                           float& footX, float& footY, float& footZ) const;

    bool isOnGround() const { return m_onGround; }
    float getGroundForce() const { return m_groundForce; }

    float attachX = 0, attachZ = 0;
    bool leftSide = false;

    // Segment lengths
    float coxaLen  = 0.15f;
    float femurLen = 0.28f;
    float tibiaLen = 0.30f;

    // Joint angles
    float swingAngle = 0;    // Coxa: forward(+)/backward(-) swing in XZ
    float liftAngle  = 0;    // Femur: lift up(+) from horizontal
    float kneeAngle  = 0;    // Tibia: bend at knee, 0=straight, positive=fold up

private:
    float m_outX = 1, m_outZ = 0;  // Outward direction
    float m_fwdX = 0, m_fwdZ = 1;  // Forward direction (perpendicular to outward)
    float m_phase = 0;
    bool m_onGround = false;
    float m_groundForce = 0;
    int m_legIndex = 0;
};
