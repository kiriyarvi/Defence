#pragma once
#include "gui/widget.h"
#include <string>


class Label: public Widget {
public:
    Label(bool inline_text, size_t size, sf::Font* font);
    static std::unique_ptr<Label> create(bool inline_text = false, size_t size = 30, sf::Font* font = nullptr) { return std::make_unique<Label>(inline_text, size, font); }
    void draw(const glm::vec2& position_transform, sf::RenderWindow& window) override;
    void add_text(const std::string& text, sf::Color color = sf::Color::White, sf::Text::Style style = sf::Text::Style::Regular);
    void clear();
    static sf::Color gold_color;

    std::function<float()> width_func;
private:
    void add_paragraph_text_rule();
    void add_inline_text_rule();
    struct FontInfo {
        sf::Font* font;
        size_t size;
        float space_width;
        float top_line; //линия над всеми символами (относительно baseline)
        float sfml_offset; //смещение по y, которое придает тексту sfml
        float line_spacing;
        sf::FloatRect get_text_bounds(const std::basic_string<sf::Uint32>& text, sf::Text::Style style);
    } m_font_info;

    struct Word {
        std::basic_string<sf::Uint32> word;
        sf::Color color;
        sf::Text::Style style;
        float width = 0.f;
        bool break_line = false;
        bool compulsory_line_break = false;
    };
    std::list<Word> m_text;

    float m_cached_width = 0;
    float m_cached_height = 0;
    bool m_invalidated = true;
    bool m_inline_text;
};

