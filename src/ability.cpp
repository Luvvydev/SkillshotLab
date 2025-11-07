#include "ability.hpp"

namespace SkillshotLab {

// This is a very small and intentionally simple predictor.
// It ignores acceleration and just assumes the target keeps its current velocity.
Vec2 predict_impact_point(const Vec2& caster_pos,
                          const Vec2& target_pos,
                          const Vec2& target_vel,
                          const AbilityDefinition& def) {
    // First guess: time until missile reaches current target position
    float distance_now = distance(caster_pos, target_pos);
    float travel_time = distance_now / def.speed;
    float t = def.cast_delay + travel_time;

    // Predict target future position
    Vec2 future_pos = target_pos + target_vel * t;

    // Recompute travel time using future position for a single refinement step
    float distance_future = distance(caster_pos, future_pos);
    float travel_time_refined = distance_future / def.speed;
    float t_refined = def.cast_delay + travel_time_refined;

    Vec2 refined_pos = target_pos + target_vel * t_refined;
    return refined_pos;
}

} // namespace SkillshotLab
