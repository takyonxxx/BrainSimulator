#pragma once
#include "platform.h"
#include "food.h"
#include "creature/creature.h"
#include <vector>

class World {
public:
    World(float size = 10.0f);

    void initialize();
    void update(float dt, Creature& creature);
    void reset();

    float size() const { return m_size; }
    const std::vector<Food>& foods() const { return m_foods; }
    const std::vector<Obstacle>& obstacles() const { return m_obstacles; }

    // Stats
    int totalFoodEaten() const { return m_totalFoodEaten; }
    float simulationTime() const { return m_simTime; }

    std::vector<std::pair<float,float>> getFoodPositions() const;
    std::vector<std::pair<float,float>> getObstaclePositions() const;

private:
    float m_size;
    std::vector<Food> m_foods;
    std::vector<Obstacle> m_obstacles;
    float m_simTime = 0;
    int m_totalFoodEaten = 0;

    void checkFoodConsumption(Creature& creature);
    void respawnFood(float dt);
    void keepInBounds(Creature& creature);
};
