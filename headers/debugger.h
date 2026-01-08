#pragma once
#include <list>
#include <glm/vec2.hpp>
#include <SFML/Graphics.hpp>


class Debugger {
public:
    static Debugger& Instance() {
        static Debugger instance;
        return instance;
    }

    // Удаляем копирование и перемещение
    Debugger(const Debugger&) = delete;
    Debugger& operator=(const Debugger&) = delete;
    Debugger(Debugger&&) = delete;
    Debugger& operator=(Debugger&&) = delete;

    void add_line(const glm::vec2& from, const glm::vec2& to, sf::Color color);
    void add_indicator(uint32_t enemy_id, float p);
    void draw(sf::RenderWindow& window);
     
private:
    Debugger() = default;
private:
    struct Line {
        glm::vec2 from;
        glm::vec2 to;
        sf::Color color;
    };
    std::list<Line> m_lines;

    struct EnemyIndicators {
        uint32_t enemy_id;
        glm::vec2 enemy_pos;
        std::list<float> progresses;
    };

    std::list<EnemyIndicators> m_enemies_indicators;

};
