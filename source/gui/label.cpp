#include "gui/label.h"
#include "resource_manager.h"
#include <SFML/System/Utf.hpp>

sf::Color Label::gold_color = sf::Color(255, 211, 3);

sf::FloatRect Label::FontInfo::get_text_bounds(const std::basic_string<sf::Uint32>& text, sf::Text::Style style) {
    sf::Text widget(sf::String(text), *font, size);
    widget.setStyle(style);
    return widget.getLocalBounds();
}

Label::Label(bool inline_text, size_t size, sf::Font* font) {
    layout.padding = { 3,3,3,3 }; //default padding.
    m_inline_text = inline_text;
    m_font_info.font = font ? font:  &ResourceManager::Instance().GOSTtypeA_font;
    m_font_info.size = size;
    m_font_info.space_width = m_font_info.font->getGlyph(' ', m_font_info.size, true).advance;
    auto bounds = m_font_info.get_text_bounds({ sf::Uint32('A'), sf::Uint32('g') }, sf::Text::Style::Regular);
    m_font_info.top_line = bounds.height;
    m_font_info.sfml_offset = m_font_info.get_text_bounds({ sf::Uint32('A') }, sf::Text::Style::Regular).top;
    m_font_info.line_spacing = m_font_info.font->getLineSpacing(m_font_info.size);
    m_font_info.font->setSmooth(false);
    if (inline_text)
        add_inline_text_rule();
    else
        add_paragraph_text_rule();
}

void Label::add_paragraph_text_rule() {
    add_rule(Property::HEIGHT, [this](Layout& layout) {
        float content_width = layout.width - layout.padding.left - layout.padding.right;
        float line_width = 0;
        int lines = 1;
        std::list<Word>::iterator prev_it = m_text.end();
        for (auto word_it = m_text.begin(); word_it != m_text.end(); ++word_it) {
            word_it->break_line = false;
            bool new_line = false; //начинать ли новую строку
            if (prev_it != m_text.end() && prev_it->compulsory_line_break == true)
                new_line = true; //если после предыдущего слова обязательная новая строка, начинаем новую строку.
            else {
                //вычисляем line_width так, как если бы очередное слово располагалось в той же строке
                line_width += word_it->width;
                if (word_it != m_text.begin())
                    line_width += m_font_info.space_width;
                if (line_width >= content_width)
                    new_line = true;
            }
            if (new_line) {
                if (prev_it != m_text.end())
                    prev_it->break_line = true;
                line_width = word_it->width;
                ++lines;
            }
            prev_it = word_it;
        }
        layout.height = (lines - 1) * m_font_info.line_spacing + m_font_info.top_line + layout.padding.top + layout.padding.bottom;
    }, { {this, Property::WIDTH} });
}

void Label::add_inline_text_rule() {
    add_rule(Property::SIZE, [this](Layout& layout) {
        float max_line_width = 0;
        float line_width = 0;
        int lines = 1;
        std::list<Word>::iterator prev_it = m_text.end();
        for (auto word_it = m_text.begin(); word_it != m_text.end(); ++word_it) {
            word_it->break_line = false;
            if (prev_it != m_text.end() && prev_it->compulsory_line_break == true) {
                prev_it->break_line = true;
                max_line_width = std::max(line_width, max_line_width);
                line_width = word_it->width;
                ++lines;
            }
            else {
                line_width += word_it->width;
                if (prev_it != m_text.end())
                    line_width += m_font_info.space_width;
            }
            prev_it = word_it;
        }
        max_line_width = std::max(line_width, max_line_width);
        layout.width = max_line_width + layout.padding.left + layout.padding.right;
        layout.height = (lines - 1) * m_font_info.line_spacing + m_font_info.top_line + layout.padding.top + layout.padding.bottom;
    },{});
}

void Label::add_text(const std::string& text, sf::Color color, sf::Text::Style style) {
    //Переводим в UTF-32
    std::basic_string<sf::Uint32> codepoints_string;
    auto begin = text.begin();
    auto end = text.end();
    while (begin < end)
    {
        sf::Uint32 codepoint;
        begin = sf::Utf8::decode(begin, end, codepoint);
        codepoints_string += codepoint;
    }
    std::basic_string<sf::Uint32> current;


    auto flush = [&](bool compulsory) {
        if (compulsory || !current.empty()) {
            auto bounds = m_font_info.get_text_bounds(current, style);
            m_text.push_back({ current, color, style,  bounds.width + bounds.left, false });
            current.clear();
        }
    };

    for (sf::Uint32 c : codepoints_string) {
        switch (c) {
        case ' ':
            flush(false);
            break;
        case '\n':
            flush(true);
            m_text.back().compulsory_line_break = true;
            break;
        default:
            current.push_back(c);
        }
    }
    flush(false);
    if (m_inline_text)
        invalidate(Property::SIZE);
    else
        invalidate(Property::HEIGHT);
}

void Label::clear() {
    m_text.clear();
    if (m_inline_text)
        invalidate(Property::SIZE);
    else
        invalidate(Property::HEIGHT);
}


void Label::draw(const glm::vec2& position_transform, sf::RenderWindow& window) {
    sf::Text widget("", *m_font_info.font, m_font_info.size);

    sf::Vector2f text_start{ position_transform.x + layout.x + layout.padding.left, position_transform.y + layout.y + layout.padding.top };

    int line = 0;
    float x_offset = 0;
 
    for (auto& word : m_text) {
        widget.setString(sf::String(word.word));
        widget.setColor(word.color);
        widget.setStyle(word.style);
        widget.setPosition(text_start + sf::Vector2f{ x_offset, line * m_font_info.line_spacing - m_font_info.sfml_offset});
        window.draw(widget);
        if (word.break_line) {
            ++line;
            x_offset = 0;
        }
        else {
            x_offset += word.width;
            x_offset += m_font_info.space_width;
        }
    }
}
