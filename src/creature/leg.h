#pragma once
#include "platform.h"
#include <cmath>

// Insect leg with 3 segments: coxa (hip), femur (upper), tibia (lower)
// Coordinate system: X=right, Y=up, Z=forward (creature local space)
// Coxa rotates in XZ plane (swing forward/backward)
// Femur rotates in vertical plane (lift up/down)
// Tibia rotates in vertical plane (extend/fold)

struct LegSegment {
    float length;
    float angle;       // Current angle (radians)
    float targetAngle; // Target for smooth interpolation
};

class Leg {
public:
    Leg() = default;

    // attachX/Z: where leg connects to body (local coords)
    // outAngle: direction leg points outward (radians in XZ plane, 0=+X, PI/2=+Z)
    // left: which side
    void init(float attachX, float attachZ, float outAngle, bool left, int legIndex);

    void update(float dt, float motorSignal);
    void applyGroundContact(float groundY);

    // Get world-space positions of each joint (relative to body center)
    void getJointPositions(float& hipX, float& hipY, float& hipZ,
                           float& kneeX, float& kneeY, float& kneeZ,
                           float& footX, float& footY, float& footZ) const;

    float getGroundForce() const { return m_groundForce; }
    bool isOnGround() const { return m_onGround; }

    // Segment data
    LegSegment coxa  = {0.12f, 0, 0};  // Short hip
    LegSegment femur = {0.20f, 0, 0};  // Upper leg
    LegSegment tibia = {0.22f, 0, 0};  // Lower leg

    float attachX = 0, attachZ = 0;
    float outwardAngle = 0;  // Direction leg points (XZ plane)
    bool leftSide = false;

private:
    float m_phase = 0;
    float m_groundForce = 0;
    bool m_onGround = false;
    int m_legIndex = 0;
};
