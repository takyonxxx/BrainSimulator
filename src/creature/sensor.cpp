#include "platform.h"
#include "sensor.h"
#include <algorithm>

SensorSystem::SensorSystem()
{
    // Touch sensors around the body
    touchSensors.push_back({TouchSensor::Nose, 0, 0, 0.4f});
    touchSensors.push_back({TouchSensor::Anterior, -0.2f, 0, 0.2f});
    touchSensors.push_back({TouchSensor::Anterior, 0.2f, 0, 0.2f});
    touchSensors.push_back({TouchSensor::Posterior, -0.1f, 0, -0.3f});
    touchSensors.push_back({TouchSensor::Posterior, 0.1f, 0, -0.3f});
    touchSensors.push_back({TouchSensor::LeftSide, -0.25f, 0, 0.0f});
    touchSensors.push_back({TouchSensor::RightSide, 0.25f, 0, 0.0f});

    for (int i = 0; i < 6; i++)
        proprioceptors.push_back(ProprioceptiveSensor(i));
}

void SensorSystem::update(float bodyX, float bodyZ, float bodyAngle,
                          const std::vector<std::pair<float,float>>& foodPositions,
                          const std::vector<std::pair<float,float>>& obstaclePositions,
                          float worldSize)
{
    float cosA = cosf(bodyAngle);
    float sinA = sinf(bodyAngle);

    // Update chemical sensors
    auto transformSensor = [&](float lx, float lz) -> std::pair<float,float> {
        return {bodyX + lx * cosA - lz * sinA,
                bodyZ + lx * sinA + lz * cosA};
    };

    auto [lx, lz] = transformSensor(leftAntenna.posX, leftAntenna.posZ);
    leftAntenna.concentration = computeChemConcentration(lx, lz, foodPositions);

    auto [rx, rz] = transformSensor(rightAntenna.posX, rightAntenna.posZ);
    rightAntenna.concentration = computeChemConcentration(rx, rz, foodPositions);

    // Update touch sensors
    for (auto& ts : touchSensors) {
        auto [wx, wz] = transformSensor(ts.posX, ts.posZ);
        ts.value = computeTouchValue(wx, wz, obstaclePositions);

        // World boundary touch
        float margin = 0.5f;
        if (wx < -worldSize + margin || wx > worldSize - margin ||
            wz < -worldSize + margin || wz > worldSize - margin) {
            ts.value = (std::max)(ts.value, 0.8f);
        }
    }
}

float SensorSystem::computeChemConcentration(float sx, float sz,
                                              const std::vector<std::pair<float,float>>& sources)
{
    float totalConc = 0;
    for (auto& [fx, fz] : sources) {
        float dx = sx - fx;
        float dz = sz - fz;
        float dist = sqrtf(dx * dx + dz * dz) + 0.01f;
        // Gaussian-like concentration field
        totalConc += expf(-dist * dist * 0.5f) * 1.5f;
        // Also add 1/r component for long-range detection
        totalConc += 0.3f / (dist + 0.5f);
    }
    return (std::clamp)(totalConc, 0.0f, 1.0f);
}

float SensorSystem::computeTouchValue(float sx, float sz,
                                       const std::vector<std::pair<float,float>>& obstacles)
{
    float maxTouch = 0;
    for (auto& [ox, oz] : obstacles) {
        float dx = sx - ox;
        float dz = sz - oz;
        float dist = sqrtf(dx * dx + dz * dz);
        if (dist < 0.5f) {
            maxTouch = (std::max)(maxTouch, 1.0f - dist / 0.5f);
        }
    }
    return maxTouch;
}

float SensorSystem::touchAnterior() const
{
    float sum = 0;
    int count = 0;
    for (auto& ts : touchSensors) {
        if (ts.region == TouchSensor::Anterior || ts.region == TouchSensor::Nose) {
            sum += ts.value;
            count++;
        }
    }
    return count > 0 ? sum / count : 0;
}

float SensorSystem::touchPosterior() const
{
    float sum = 0;
    int count = 0;
    for (auto& ts : touchSensors) {
        if (ts.region == TouchSensor::Posterior) {
            sum += ts.value;
            count++;
        }
    }
    return count > 0 ? sum / count : 0;
}

float SensorSystem::touchNose() const
{
    for (auto& ts : touchSensors) {
        if (ts.region == TouchSensor::Nose) return ts.value;
    }
    return 0;
}
