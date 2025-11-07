#include "app.hpp"
#include "scenario.hpp"
#include "geometry.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace SkillshotLab {

namespace {
sf::Vector2f world_to_screen(const Vec2& v) {
    const float offset_x = 300.0f;
    const float offset_y = 360.0f;
    const float scale = 1.0f;

    return sf::Vector2f(offset_x + v.x * scale, offset_y - v.y * scale);
}
} // namespace

void App::run() {
    sf::RenderWindow window(sf::VideoMode({1200u, 720u}), "SkillshotLab");
    window.setFramerateLimit(60);

    Scenario scenario;
    scenario.create_demo();

    sf::Font font;
    bool have_font = font.openFromFile("DejaVuSans.ttf");
    if (!have_font) {
        std::cout << "Warning: could not open DejaVuSans.ttf, stats text will not be visible\n";
    }

    sf::Text stats_text(font, "", 20);
    stats_text.setFillColor(sf::Color(230, 230, 230));

    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();
        scenario.update(dt);

        window.clear(sf::Color(10, 10, 20));

        // Caster color flashes when hit
        float flash = scenario.caster_flash_time();
        sf::Color caster_color = (flash > 0.0f)
            ? sf::Color(255, 120, 120)
            : sf::Color(180, 200, 255);

        sf::CircleShape caster_shape(9.0f);
        caster_shape.setOrigin({9.0f, 9.0f});
        caster_shape.setFillColor(caster_color);
        caster_shape.setPosition(world_to_screen(scenario.caster_pos()));
        window.draw(caster_shape);

        // Ability range
        const float range = scenario.ability().range;
        sf::CircleShape range_shape(range);
        range_shape.setOrigin({range, range});
        range_shape.setFillColor(sf::Color::Transparent);
        range_shape.setOutlineThickness(1.0f);
        range_shape.setOutlineColor(sf::Color(80, 80, 140));
        range_shape.setPosition(world_to_screen(scenario.caster_pos()));
        window.draw(range_shape);

        // Targets and predicted hit positions
        const auto& targets = scenario.targets();
        for (std::size_t i = 0; i < targets.size(); ++i) {
            const auto& t = targets[i];

            Vec2 predicted = predict_impact_point(
                scenario.caster_pos(),
                t.position,
                t.velocity,
                scenario.ability()
            );

            sf::CircleShape target_shape(12.0f);
            target_shape.setOrigin({12.0f, 12.0f});
            target_shape.setFillColor(sf::Color(220, 160, 160));
            target_shape.setPosition(world_to_screen(t.position));
            window.draw(target_shape);

            sf::CircleShape predicted_shape(5.0f);
            predicted_shape.setOrigin({5.0f, 5.0f});
            predicted_shape.setFillColor(sf::Color(120, 220, 160));
            predicted_shape.setPosition(world_to_screen(predicted));
            window.draw(predicted_shape);
        }

        // Friendly projectiles
        const auto& projectiles = scenario.projectiles();
        for (const auto& p : projectiles) {
            sf::CircleShape missile(4.0f);
            missile.setOrigin({4.0f, 4.0f});
            missile.setFillColor(sf::Color(255, 230, 120));
            missile.setPosition(world_to_screen(p.position));
            window.draw(missile);
        }

        // Enemy projectiles
        const auto& enemy_projectiles = scenario.enemy_projectiles();
        for (const auto& ep : enemy_projectiles) {
            sf::CircleShape shot(4.0f);
            shot.setOrigin({4.0f, 4.0f});
            shot.setFillColor(sf::Color(120, 200, 255));
            shot.setPosition(world_to_screen(ep.position));
            window.draw(shot);
        }

        // Explosions and prediction error ghosts
        const auto& explosions = scenario.explosions();
        for (const auto& e : explosions) {
            float t = e.time / e.duration;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            std::uint8_t ghost_alpha = static_cast<std::uint8_t>(200.0f * (1.0f - t));
            sf::CircleShape ghost(4.0f);
            ghost.setOrigin({4.0f, 4.0f});
            ghost.setFillColor(sf::Color(210, 210, 255, ghost_alpha));
            ghost.setPosition(world_to_screen(e.predicted));
            window.draw(ghost);

            sf::Vertex line[2];
            line[0].position = world_to_screen(e.predicted);
            line[0].color = sf::Color(210, 210, 255, ghost_alpha);
            line[1].position = world_to_screen(e.position);
            line[1].color = sf::Color(80, 255, 80, ghost_alpha);
            window.draw(line, 2, sf::PrimitiveType::Lines);

            float radius = 10.0f + 50.0f * t;
            sf::CircleShape boom(radius);
            boom.setOrigin({radius, radius});

            std::uint8_t alpha = static_cast<std::uint8_t>(255.0f * (1.0f - t));
            boom.setFillColor(sf::Color(180, 255, 180, alpha));
            boom.setOutlineThickness(2.0f);
            boom.setOutlineColor(sf::Color(80, 220, 80, alpha));

            boom.setPosition(world_to_screen(e.position));
            window.draw(boom);
        }

        // Stats text at top left
        int hits = scenario.hits();
        int shots = scenario.shots_fired();
        int dmg = scenario.hits_taken();

        float acc = 0.0f;
        if (shots > 0) {
            acc = static_cast<float>(hits) * 100.0f / static_cast<float>(shots);
        }

        std::ostringstream ss;
        ss.setf(std::ios::fixed);
        ss.precision(1);
        ss << "Shots " << shots
           << "   Hits " << hits
           << "   Accuracy " << acc << " percent"
           << "   DamageTaken " << dmg;

        stats_text.setString(ss.str());
        stats_text.setPosition(sf::Vector2f(20.0f, 20.0f));
        window.draw(stats_text);

        window.display();
    }
}

} // namespace SkillshotLab
