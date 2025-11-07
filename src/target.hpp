#pragma once

#include "geometry.hpp"

namespace SkillshotLab {

struct Target {
    Vec2 position;
    Vec2 velocity;

    void update(float dt) {
        position += velocity * dt;
    }
};

} // namespace SkillshotLab
