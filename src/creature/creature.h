#pragma once
#include "platform.h"
#include "brain/brain.h"
#include "creature/leg.h"
#include "creature/sensor.h"
#include <vector>
#include <array>

class Creature {
public:
    Creature();

    void initialize();
    void update(float dt, const std::vector<std::pair<float,float>>& foodPos,
                const std::vector<std::pair<float,float>>& obstaclePos,
                float worldSize);
    void reset();

    // Position and orientation
    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }
    float angle() const { return m_angle; }
    float speed() const { return m_speed; }
    float energy() const { return m_energy; }

    // Components
    Brain& brain() { return m_brain; }
    const Brain& brain() const { return m_brain; }
    const std::array<Leg, 6>& legs() const { return m_legs; }
    const SensorSystem& sensors() const { return m_sensors; }

    // Body dimensions
    static constexpr float BODY_LENGTH = 0.5f;
    static constexpr float BODY_WIDTH = 0.25f;
    static constexpr float BODY_HEIGHT = 0.12f;

    // State
    enum class Behavior { Exploring, Feeding, Avoiding, Turning, Idle };
    Behavior currentBehavior() const { return m_behavior; }
    float distanceTraveled() const { return m_distanceTraveled; }
    int foodEaten() const { return m_foodEaten; }

    void eatFood() { m_energy = (std::min)(1.0f, m_energy + 0.2f); m_foodEaten++; }

private:
    Brain m_brain;
    std::array<Leg, 6> m_legs;
    SensorSystem m_sensors;

    float m_x = 0, m_y = 0.2f, m_z = 0;
    float m_angle = 0;
    float m_speed = 0;
    float m_energy = 0.8f;
    float m_distanceTraveled = 0;
    int m_foodEaten = 0;

    Behavior m_behavior = Behavior::Exploring;

    void updatePhysics(float dt);
    void feedSensoryInput();
    void readMotorOutput(float dt);
    void updateBehaviorState();
};
