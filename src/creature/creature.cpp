#include "platform.h"
#include "creature.h"
#include <cmath>
#include <algorithm>

Creature::Creature() {}

void Creature::initialize()
{
    m_brain.initialize();

    // 6 legs: 3 pairs (front, middle, rear)
    // outDir: unit vector pointing outward from body
    struct LegDef {
        float ax, az;        // Attach point on body
        float outX, outZ;    // Outward direction
        bool left;
    };

    float hw = BODY_WIDTH * 0.5f;  // Half body width

    LegDef defs[6] = {
        // Left legs point left (-X) with slight forward/backward angle
        {-hw,  0.16f,  -1.0f,  0.4f,  true},   // L-front: left + slightly forward
        {-hw,  0.00f,  -1.0f,  0.0f,  true},   // L-mid:   straight left
        {-hw, -0.16f,  -1.0f, -0.4f,  true},   // L-rear:  left + slightly backward
        // Right legs point right (+X) with slight forward/backward angle
        { hw,  0.16f,   1.0f,  0.4f,  false},  // R-front: right + slightly forward
        { hw,  0.00f,   1.0f,  0.0f,  false},  // R-mid:   straight right
        { hw, -0.16f,   1.0f, -0.4f,  false},  // R-rear:  right + slightly backward
    };

    for (int i = 0; i < 6; i++) {
        m_legs[i].init(defs[i].ax, defs[i].az,
                       defs[i].outX, defs[i].outZ,
                       defs[i].left, i);
    }

    m_x = 0; m_z = 0; m_y = 0.2f;
    m_angle = 0;
    m_speed = 0;
    m_energy = 0.8f;
    m_distanceTraveled = 0;
    m_foodEaten = 0;
}

void Creature::update(float dt, const std::vector<std::pair<float,float>>& foodPos,
                      const std::vector<std::pair<float,float>>& obstaclePos,
                      float worldSize)
{
    // 1. Update sensors
    m_sensors.update(m_x, m_z, m_angle, foodPos, obstaclePos, worldSize);

    // 2. Feed sensory input to brain
    feedSensoryInput();

    // 3. Brain step (multiple sub-steps for neural dynamics)
    int brainSteps = 4;
    float brainDt = dt * 1000.0f / brainSteps; // Convert to ms
    for (int i = 0; i < brainSteps; i++) {
        m_brain.step(brainDt);
    }

    // 4. Read motor output from brain
    readMotorOutput(dt);

    // 5. Update physics
    updatePhysics(dt);

    // 6. Update behavior state
    updateBehaviorState();

    // 7. Energy consumption
    m_energy -= 0.001f * dt * (1.0f + m_speed * 2.0f);
    m_energy = (std::clamp)(m_energy, 0.0f, 1.0f);
}

void Creature::feedSensoryInput()
{
    // Chemosensory (food detection via antennae)
    m_brain.setChemosensoryInput(m_sensors.chemLeft(), m_sensors.chemRight());

    // Mechanosensory (touch)
    m_brain.setMechanosensoryInput(
        m_sensors.touchAnterior(),
        m_sensors.touchPosterior(),
        m_sensors.touchNose()
    );

    // Proprioceptive (body bend and leg positions)
    std::vector<float> jointAngles;
    for (auto& leg : m_legs) {
        jointAngles.push_back(leg.swingAngle);
    }
    m_brain.setProprioceptiveInput(jointAngles);

    // Thermosensory
    m_brain.setThermosensoryInput(m_sensors.temperature());
}

void Creature::readMotorOutput(float dt)
{
    float forward = m_brain.getForwardDrive();
    float backward = m_brain.getBackwardDrive();
    float turn = m_brain.getTurnSignal();

    // Net locomotion drive
    float netDrive = forward - backward;

    // Speed from neural output - more responsive
    float targetSpeed = netDrive * 1.5f;
    m_speed += (targetSpeed - m_speed) * 8.0f * dt;
    m_speed = (std::clamp)(m_speed, -1.5f, 3.0f);

    // Turning from neural output - more responsive
    float turnRate = turn * 4.0f;
    m_angle += turnRate * dt;

    // Keep angle in [-PI, PI]
    while (m_angle > M_PI) m_angle -= 2.0f * (float)M_PI;
    while (m_angle < -M_PI) m_angle += 2.0f * (float)M_PI;

    // Update legs with motor signal
    float legSignal = std::abs(netDrive) > 0.02f ? netDrive : 0.15f;
    for (int i = 0; i < 6; i++) {
        float signal = legSignal;
        // Add turn differential to legs
        if (i % 2 == 0) signal += turn * 0.4f;  // Left legs
        else signal -= turn * 0.4f;              // Right legs
        m_legs[i].update(dt, signal);
    }
}

void Creature::updatePhysics(float dt)
{
    // Apply locomotion
    float dx = sinf(m_angle) * m_speed * dt;
    float dz = cosf(m_angle) * m_speed * dt;

    m_x += dx;
    m_z += dz;
    m_distanceTraveled += sqrtf(dx * dx + dz * dz);

    // Ground contact - keep body above ground
    float groundY = 0.0f;
    int groundLegs = 0;
    for (auto& leg : m_legs) {
        leg.applyGroundContact(groundY);
        if (leg.isOnGround()) groundLegs++;
    }

    // Body height based on leg support
    float targetY = 0.15f + 0.05f * (groundLegs / 6.0f);
    m_y += (targetY - m_y) * 5.0f * dt;

    // Subtle body oscillation during walking
    if (std::abs(m_speed) > 0.1f) {
        float walkCycle = fmodf(m_distanceTraveled * 8.0f, 2.0f * M_PI);
        m_y += sinf(walkCycle) * 0.005f;
    }
}

void Creature::updateBehaviorState()
{
    float feeding = m_brain.getFeedingSignal();
    float forward = m_brain.getForwardDrive();
    float backward = m_brain.getBackwardDrive();
    float turn = std::abs(m_brain.getTurnSignal());

    if (feeding > 0.3f && m_sensors.chemLeft() + m_sensors.chemRight() > 0.5f)
        m_behavior = Behavior::Feeding;
    else if (backward > forward && backward > 0.2f)
        m_behavior = Behavior::Avoiding;
    else if (turn > 0.3f)
        m_behavior = Behavior::Turning;
    else if (forward > 0.1f)
        m_behavior = Behavior::Exploring;
    else
        m_behavior = Behavior::Idle;
}

void Creature::reset()
{
    m_brain.reset();
    m_x = 0; m_z = 0; m_y = 0.2f;
    m_angle = 0;
    m_speed = 0;
    m_energy = 0.8f;
    m_distanceTraveled = 0;
    m_foodEaten = 0;
    m_behavior = Behavior::Exploring;
}
