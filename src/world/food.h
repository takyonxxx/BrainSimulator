#pragma once
#include "platform.h"
#include <vector>
#include <cmath>

struct Food {
    float x, z;
    float amount;     // 0-1
    float respawnTimer;
    bool active;

    Food(float x_, float z_) : x(x_), z(z_), amount(1.0f), respawnTimer(0), active(true) {}
};

struct Obstacle {
    float x, z;
    float radius;

    Obstacle(float x_, float z_, float r) : x(x_), z(z_), radius(r) {}
};
