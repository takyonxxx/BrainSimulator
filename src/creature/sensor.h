#pragma once
#include "platform.h"
#include <cmath>
#include <vector>

struct ChemicalSensor {
    float posX, posZ;      // Position relative to body (left/right antennae)
    float concentration;    // Detected chemical level
    float sensitivity;

    ChemicalSensor(float x, float z, float sens = 1.0f)
        : posX(x), posZ(z), concentration(0), sensitivity(sens) {}
};

struct TouchSensor {
    enum Region { Anterior, Posterior, LeftSide, RightSide, Nose };
    Region region;
    float value;
    float posX, posY, posZ;

    TouchSensor(Region r, float x, float y, float z)
        : region(r), value(0), posX(x), posY(y), posZ(z) {}
};

struct ProprioceptiveSensor {
    int legIndex;
    float jointAngle;
    float groundContact;

    ProprioceptiveSensor(int leg) : legIndex(leg), jointAngle(0), groundContact(0) {}
};

class SensorSystem {
public:
    SensorSystem();

    void update(float bodyX, float bodyZ, float bodyAngle,
                const std::vector<std::pair<float,float>>& foodPositions,
                const std::vector<std::pair<float,float>>& obstaclePositions,
                float worldSize);

    // Chemical sensors (left/right antennae)
    ChemicalSensor leftAntenna{-0.15f, 0.3f, 1.0f};
    ChemicalSensor rightAntenna{0.15f, 0.3f, 1.0f};

    // Touch sensors
    std::vector<TouchSensor> touchSensors;

    // Proprioceptive (6 legs)
    std::vector<ProprioceptiveSensor> proprioceptors;

    // Computed sensory values for brain
    float chemLeft() const { return leftAntenna.concentration; }
    float chemRight() const { return rightAntenna.concentration; }
    float touchAnterior() const;
    float touchPosterior() const;
    float touchNose() const;
    float temperature() const { return m_temperature; }

private:
    float m_temperature = 0.5f;

    float computeChemConcentration(float sensorWorldX, float sensorWorldZ,
                                   const std::vector<std::pair<float,float>>& sources);
    float computeTouchValue(float sensorWorldX, float sensorWorldZ,
                           const std::vector<std::pair<float,float>>& obstacles);
};
