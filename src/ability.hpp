#pragma once

#include "geometry.hpp"

namespace SkillshotLab {

// Very simple linear skillshot definition.
struct AbilityDefinition {
    float range{800.0f};
    float speed{1200.0f};   // units per second
    float cast_delay{0.25f}; // seconds
    float radius{80.0f};
};

struct AbilityCast {
    Vec2 start_pos;
    Vec2 cast_dir;
    float cast_time{0.0f};
    AbilityDefinition def;
};

// Predict where to aim so the missile reaches the target after delay and travel time.
Vec2 predict_impact_point(const Vec2& caster_pos,
                          const Vec2& target_pos,
                          const Vec2& target_vel,
                          const AbilityDefinition& def);

} // namespace SkillshotLab
