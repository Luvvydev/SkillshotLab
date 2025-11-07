#pragma once

#include "ability.hpp"
#include "target.hpp"
#include <vector>

namespace SkillshotLab {

struct ScenarioSettings {
    int   target_count{4};
    float cast_interval{0.6f};

    float ability_range{900.0f};
    float ability_speed{1600.0f};
    float ability_radius{80.0f};

    float target_min_speed{80.0f};
    float target_max_speed{220.0f};

    float caster_speed{220.0f};
    float enemy_cast_interval{1.2f};
    float enemy_projectile_speed{900.0f};
    float caster_hit_radius{18.0f};
};

struct Projectile {
    Vec2 position;
    Vec2 velocity;
    Vec2 predicted;
    float life{0.0f};
};

struct EnemyProjectile {
    Vec2 position;
    Vec2 velocity;
    float life{0.0f};
};

struct Explosion {
    Vec2 position;
    Vec2 predicted;
    float time{0.0f};
    float duration{0.4f};
};

class Scenario {
public:
    void create_demo();
    void update(float dt);
    void print_state() const;

    const Vec2& caster_pos() const { return m_caster_pos; }
    const AbilityDefinition& ability() const { return m_ability; }
    const std::vector<Target>& targets() const { return m_targets; }
    const std::vector<Projectile>& projectiles() const { return m_projectiles; }
    const std::vector<EnemyProjectile>& enemy_projectiles() const { return m_enemy_projectiles; }
    const std::vector<Explosion>& explosions() const { return m_explosions; }
    const ScenarioSettings& settings() const { return m_settings; }

    int hits() const { return m_hits; }
    int shots_fired() const { return m_shots_fired; }
    int hits_taken() const { return m_hits_taken; }
    float caster_flash_time() const { return m_caster_hit_flash_time; }

private:
    Vec2 m_caster_pos{0.0f, 0.0f};
    Vec2 m_caster_vel{0.0f, 0.0f};
    AbilityDefinition m_ability;

    std::vector<Target> m_targets;
    std::vector<Projectile> m_projectiles;
    std::vector<EnemyProjectile> m_enemy_projectiles;
    std::vector<Explosion> m_explosions;

    ScenarioSettings m_settings{};
    float m_time_since_last_cast{0.0f};
    float m_enemy_time_since_last_cast{0.0f};

    int m_hits{0};
    int m_shots_fired{0};
    int m_hits_taken{0};
    float m_caster_hit_flash_time{0.0f};
};

} // namespace SkillshotLab
