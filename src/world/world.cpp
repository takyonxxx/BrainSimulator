#include "platform.h"
#include "world.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

World::World(float size) : m_size(size) {}

void World::initialize()
{
    m_foods.clear();
    m_obstacles.clear();
    m_simTime = 0;
    m_totalFoodEaten = 0;

    // Scatter food sources
    srand(42);
    for (int i = 0; i < 12; i++) {
        float fx = ((float)(rand() % 1000) / 500.0f - 1.0f) * m_size * 0.8f;
        float fz = ((float)(rand() % 1000) / 500.0f - 1.0f) * m_size * 0.8f;
        m_foods.emplace_back(fx, fz);
    }

    // Place obstacles
    for (int i = 0; i < 8; i++) {
        float ox = ((float)(rand() % 1000) / 500.0f - 1.0f) * m_size * 0.6f;
        float oz = ((float)(rand() % 1000) / 500.0f - 1.0f) * m_size * 0.6f;
        float r = 0.3f + (float)(rand() % 100) / 200.0f;
        // Don't place too close to origin (creature start)
        if (sqrtf(ox * ox + oz * oz) > 2.0f) {
            m_obstacles.emplace_back(ox, oz, r);
        }
    }
}

void World::update(float dt, Creature& creature)
{
    m_simTime += dt;
    checkFoodConsumption(creature);
    respawnFood(dt);
    keepInBounds(creature);
}

void World::checkFoodConsumption(Creature& creature)
{
    float cx = creature.x();
    float cz = creature.z();
    float eatRadius = 0.4f;

    for (auto& food : m_foods) {
        if (!food.active) continue;
        float dx = cx - food.x;
        float dz = cz - food.z;
        float dist = sqrtf(dx * dx + dz * dz);
        if (dist < eatRadius) {
            food.amount -= 0.02f;
            if (food.amount <= 0) {
                food.active = false;
                food.respawnTimer = 10.0f + (float)(rand() % 100) / 10.0f;
                creature.eatFood();
                m_totalFoodEaten++;
            }
        }
    }
}

void World::respawnFood(float dt)
{
    for (auto& food : m_foods) {
        if (!food.active) {
            food.respawnTimer -= dt;
            if (food.respawnTimer <= 0) {
                food.active = true;
                food.amount = 1.0f;
                // Respawn at new random location
                food.x = ((float)(rand() % 1000) / 500.0f - 1.0f) * m_size * 0.8f;
                food.z = ((float)(rand() % 1000) / 500.0f - 1.0f) * m_size * 0.8f;
            }
        }
    }
}

void World::keepInBounds(Creature& creature)
{
    // World boundaries handled by sensor touch detection
    // The brain's avoidance circuit handles the response
}

std::vector<std::pair<float,float>> World::getFoodPositions() const
{
    std::vector<std::pair<float,float>> pos;
    for (auto& f : m_foods) {
        if (f.active) pos.push_back({f.x, f.z});
    }
    return pos;
}

std::vector<std::pair<float,float>> World::getObstaclePositions() const
{
    std::vector<std::pair<float,float>> pos;
    for (auto& o : m_obstacles) {
        pos.push_back({o.x, o.z});
    }
    return pos;
}

void World::reset()
{
    initialize();
}
