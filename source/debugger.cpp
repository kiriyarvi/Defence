#include <debugger.h>
#include <enemy_manager.h>

void Debugger::add_line(const glm::vec2& from, const glm::vec2& to, sf::Color color) {
    m_lines.push_back({ from, to, color });
}

void Debugger::add_indicator(uint32_t enemy_id, float p) {
    auto it = std::find_if(m_enemies_indicators.begin(), m_enemies_indicators.end(), [&](EnemyIndicators& ind) {return ind.enemy_id == enemy_id; });
    if (it != m_enemies_indicators.end()) {
        it->progresses.push_back(p);
    }
    else {
        EnemyIndicators i;
        i.enemy_id = enemy_id;
        i.enemy_pos = EnemyManager::Instance().get_enemy_by_id(enemy_id)->position;
        i.progresses.push_back(p);
        m_enemies_indicators.push_back(i);
    }
}

void Debugger::draw(sf::RenderWindow& window) {
    sf::VertexArray line(sf::PrimitiveType::Lines);
    for (auto& l : m_lines) {
        line.append(sf::Vertex(sf::Vector2f(l.from.x, l.from.y), l.color));
        line.append(sf::Vertex(sf::Vector2f(l.to.x, l.to.y), l.color));
    }
    window.draw(line);

    m_lines.clear();

    HealthIndicator indicator;
    indicator.fill_color = sf::Color::Blue;
    indicator.width = 10;
    for (auto& e : m_enemies_indicators) {
        auto it = e.progresses.begin();
        for (size_t i = 0; i < e.progresses.size(); ++i, ++it) {
            indicator.draw(window, e.enemy_pos.x, e.enemy_pos.y + 2.25 * i, 1., *it);
        }
    }
    m_enemies_indicators.clear();

}
