#include "scenario.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <cmath>

namespace SkillshotLab {

// Simple world bounds so things stay on screen
static constexpr float WORLD_LEFT   = 0.0f;
static constexpr float WORLD_RIGHT  = 900.0f;
static constexpr float WORLD_BOTTOM = -300.0f;
static constexpr float WORLD_TOP    = 300.0f;

void Scenario::create_demo() {
    m_caster_pos = Vec2{0.0f, 0.0f};
    m_caster_vel = Vec2{0.0f, 0.0f};

    // Default preset
    m_settings.target_count           = 4;
    m_settings.cast_interval          = 0.6f;
    m_settings.ability_range          = 900.0f;
    m_settings.ability_speed          = 1600.0f;
    m_settings.ability_radius         = 80.0f;
    m_settings.target_min_speed       = 80.0f;
    m_settings.target_max_speed       = 220.0f;
    m_settings.caster_speed           = 220.0f;
    m_settings.enemy_cast_interval    = 1.2f;
    m_settings.enemy_projectile_speed = 900.0f;
    m_settings.caster_hit_radius      = 18.0f;

    m_ability.range = m_settings.ability_range;
    m_ability.speed = m_settings.ability_speed;
    m_ability.cast_delay = 0.25f;
    m_ability.radius = m_settings.ability_radius;

    m_targets.clear();
    m_projectiles.clear();
    m_enemy_projectiles.clear();
    m_explosions.clear();

    m_time_since_last_cast = 0.0f;
    m_enemy_time_since_last_cast = 0.0f;
    m_hits = 0;
    m_shots_fired = 0;
    m_hits_taken = 0;
    m_caster_hit_flash_time = 0.0f;

    // Seed some initial zombies
    for (int i = 0; i < m_settings.target_count; ++i) {
        Target t;
        float base_x = 350.0f + 110.0f * static_cast<float>(i);
        float base_y = (i % 2 == 0) ? 120.0f : -120.0f;
        t.position = Vec2{base_x, base_y};

        float vx = (i % 2 == 0) ? 120.0f : -120.0f;
        float vy = (i % 3 == 0) ? 60.0f : -40.0f;
        t.velocity = Vec2{vx, vy};

        m_targets.push_back(t);
    }
}

void Scenario::update(float dt) {
    // Move targets and bounce inside world bounds
    for (auto& t : m_targets) {
        t.update(dt);

        if (t.position.x < WORLD_LEFT) {
            t.position.x = WORLD_LEFT;
            if (t.velocity.x < 0.0f) {
                t.velocity.x = -t.velocity.x;
            }
        } else if (t.position.x > WORLD_RIGHT) {
            t.position.x = WORLD_RIGHT;
            if (t.velocity.x > 0.0f) {
                t.velocity.x = -t.velocity.x;
            }
        }

        if (t.position.y < WORLD_BOTTOM) {
            t.position.y = WORLD_BOTTOM;
            if (t.velocity.y < 0.0f) {
                t.velocity.y = -t.velocity.y;
            }
        } else if (t.position.y > WORLD_TOP) {
            t.position.y = WORLD_TOP;
            if (t.velocity.y > 0.0f) {
                t.velocity.y = -t.velocity.y;
            }
        }
    }

    // Evade enemy projectiles by moving caster perpendicular to the most dangerous shot
    Vec2 move_dir{0.0f, 0.0f};
    float best_time = 1e9f;

    for (const auto& ep : m_enemy_projectiles) {
        if (ep.life <= 0.0f) {
            continue;
        }

        Vec2 to_caster = m_caster_pos - ep.position;
        float speed = length(ep.velocity);
        if (speed <= 1e-3f) {
            continue;
        }

        Vec2 dir = Vec2{ep.velocity.x / speed, ep.velocity.y / speed};
        float along = dot(to_caster, dir);
        if (along <= 0.0f) {
            continue;
        }

        Vec2 closest_point = ep.position + dir * along;
        float dist_to_path = distance(closest_point, m_caster_pos);

        float threat_radius = 120.0f;
        if (dist_to_path > threat_radius) {
            continue;
        }

        float time_to_closest = along / speed;
        if (time_to_closest < best_time) {
            best_time = time_to_closest;

            float side = cross(to_caster, dir);
            if (side >= 0.0f) {
                move_dir = Vec2{-dir.y, dir.x};
            } else {
                move_dir = Vec2{dir.y, -dir.x};
            }
        }
    }

    if (length(move_dir) > 1e-3f) {
        move_dir = normalize(move_dir);
    }

    m_caster_vel = move_dir * m_settings.caster_speed;
    m_caster_pos += m_caster_vel * dt;

    // Keep caster in a safe region near the left
    float min_x = WORLD_LEFT + 80.0f;
    float max_x = WORLD_LEFT + 260.0f;
    float min_y = WORLD_BOTTOM + 140.0f;
    float max_y = WORLD_TOP - 140.0f;

    if (m_caster_pos.x < min_x) m_caster_pos.x = min_x;
    if (m_caster_pos.x > max_x) m_caster_pos.x = max_x;
    if (m_caster_pos.y < min_y) m_caster_pos.y = min_y;
    if (m_caster_pos.y > max_y) m_caster_pos.y = max_y;

    // Friendly projectiles using prediction
    m_time_since_last_cast += dt;
    const float cast_interval = m_settings.cast_interval;

    if (m_time_since_last_cast >= cast_interval && !m_targets.empty()) {
        m_time_since_last_cast = 0.0f;

        for (const auto& t : m_targets) {
            Vec2 predicted = predict_impact_point(
                m_caster_pos,
                t.position,
                t.velocity,
                m_ability
            );

            Vec2 dir = predicted - m_caster_pos;
            float dist = length(dir);
            if (dist > m_ability.range && dist > 1e-3f) {
                dir = dir * (m_ability.range / dist);
                predicted = m_caster_pos + dir;
                dist = m_ability.range;
            }

            if (dist <= 1e-3f) {
                continue;
            }

            Vec2 shot_dir = normalize(predicted - m_caster_pos);

            Projectile p;
            p.position = m_caster_pos;
            p.velocity = shot_dir * m_ability.speed;
            p.predicted = predicted;
            p.life = (dist / m_ability.speed) + 0.3f;

            m_projectiles.push_back(p);
            ++m_shots_fired;
        }
    }

    // Enemies shoot back at the caster
    m_enemy_time_since_last_cast += dt;
    if (m_enemy_time_since_last_cast >= m_settings.enemy_cast_interval && !m_targets.empty()) {
        m_enemy_time_since_last_cast = 0.0f;

        for (const auto& t : m_targets) {
            Vec2 to_caster = m_caster_pos - t.position;
            float dist = length(to_caster);
            if (dist <= 1e-3f) {
                continue;
            }

            Vec2 dir = normalize(to_caster);

            EnemyProjectile ep;
            ep.position = t.position;
            ep.velocity = dir * m_settings.enemy_projectile_speed;
            ep.life = (m_settings.ability_range / m_settings.enemy_projectile_speed) + 0.6f;

            m_enemy_projectiles.push_back(ep);
        }
    }

    // Random generator for respawns
    static std::mt19937 rng{std::random_device{}()};

    std::uniform_real_distribution<float> x_dist(
        WORLD_LEFT + 120.0f,
        WORLD_RIGHT - 120.0f
    );
    std::uniform_real_distribution<float> y_dist(
        WORLD_BOTTOM + 80.0f,
        WORLD_TOP - 80.0f
    );
    std::uniform_real_distribution<float> speed_dist(
        m_settings.target_min_speed,
        m_settings.target_max_speed
    );
    std::uniform_real_distribution<float> angle_dist(0.0f, 6.2831853f);

    // Move friendly projectiles, handle hits, and kill out of bounds
    for (auto& p : m_projectiles) {
        if (p.life <= 0.0f) {
            continue;
        }

        p.position += p.velocity * dt;
        p.life -= dt;

        float d_from_caster = distance(m_caster_pos, p.position);
        if (d_from_caster > m_ability.range + 50.0f) {
            p.life = 0.0f;
        }

        if (p.position.x < WORLD_LEFT - 50.0f || p.position.x > WORLD_RIGHT + 50.0f ||
            p.position.y < WORLD_BOTTOM - 50.0f || p.position.y > WORLD_TOP + 50.0f) {
            p.life = 0.0f;
        }

        if (p.life <= 0.0f) {
            continue;
        }

        // Collision with targets
        for (auto& t : m_targets) {
            float d_to_target = distance(p.position, t.position);
            if (d_to_target <= m_ability.radius) {
                Explosion e;
                e.position = t.position;
                e.predicted = p.predicted;
                e.time = 0.0f;
                e.duration = 0.4f;
                m_explosions.push_back(e);

                ++m_hits;

                t.position = Vec2{
                    x_dist(rng),
                    y_dist(rng)
                };

                float speed = speed_dist(rng);
                float angle = angle_dist(rng);
                t.velocity = Vec2{
                    std::cos(angle) * speed,
                    std::sin(angle) * speed
                };

                p.life = 0.0f;
                break;
            }
        }
    }

    // Remove dead friendly projectiles
    m_projectiles.erase(
        std::remove_if(
            m_projectiles.begin(),
            m_projectiles.end(),
            [](const Projectile& p) { return p.life <= 0.0f; }),
        m_projectiles.end()
    );

    // Move enemy projectiles and check for caster hits
    for (auto& ep : m_enemy_projectiles) {
        if (ep.life <= 0.0f) {
            continue;
        }

        ep.position += ep.velocity * dt;
        ep.life -= dt;

        float d_to_caster = distance(ep.position, m_caster_pos);
        if (d_to_caster <= m_settings.caster_hit_radius) {
            ++m_hits_taken;
            m_caster_hit_flash_time = 0.25f;
            ep.life = 0.0f;
            continue;
        }

        if (ep.position.x < WORLD_LEFT - 50.0f || ep.position.x > WORLD_RIGHT + 50.0f ||
            ep.position.y < WORLD_BOTTOM - 50.0f || ep.position.y > WORLD_TOP + 50.0f) {
            ep.life = 0.0f;
        }
    }

    m_enemy_projectiles.erase(
        std::remove_if(
            m_enemy_projectiles.begin(),
            m_enemy_projectiles.end(),
            [](const EnemyProjectile& ep) { return ep.life <= 0.0f; }),
        m_enemy_projectiles.end()
    );

    // Update and remove finished explosions
    for (auto& e : m_explosions) {
        e.time += dt;
    }

    m_explosions.erase(
        std::remove_if(
            m_explosions.begin(),
            m_explosions.end(),
            [](const Explosion& e) { return e.time >= e.duration; }),
        m_explosions.end()
    );

    // Reduce caster hit flash timer
    if (m_caster_hit_flash_time > 0.0f) {
        m_caster_hit_flash_time -= dt;
        if (m_caster_hit_flash_time < 0.0f) {
            m_caster_hit_flash_time = 0.0f;
        }
    }
}

void Scenario::print_state() const {
    std::cout << std::fixed << std::setprecision(2);

    for (std::size_t i = 0; i < m_targets.size(); ++i) {
        const auto& t = m_targets[i];

        Vec2 predicted = predict_impact_point(
            m_caster_pos,
            t.position,
            t.velocity,
            m_ability
        );

        float dist = distance(m_caster_pos, predicted);

        std::cout << "Target " << i << "\n";
        std::cout << "  pos      " << t.position << "\n";
        std::cout << "  vel      " << t.velocity << "\n";
        std::cout << "  predicted " << predicted << "  distance " << dist << "\n";
    }
}

} // namespace SkillshotLab
