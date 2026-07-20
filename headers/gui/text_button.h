#pragma once
#include "gui/widget.h"

class TextButton : public Widget {
public:
    TextButton(const std::string& text);
    void enable(bool enable) { m_enabled = enable; }
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
    Query on_event(EventContext event_context);
    void set_on_clicked(const std::function<void()>& on_click) { m_on_click = on_click; }
    void set_on_hovered(const std::function<void()>& on_hovered) { m_on_hovered = on_hovered; }
    void set_on_unhovered(const std::function<void()>& on_unhovered) { m_on_unhovered = on_unhovered; }
private:
    void set_button_size(float button_size, bool forced = false);
    float m_button_size = 0; //размеры 1, 1.5, 2, 2.5, 3 ,4
    std::function<void()> m_on_click;
    std::function<void()> m_on_hovered;
    std::function<void()> m_on_unhovered;
    sf::Sprite m_button_sprite;
    bool m_enabled = true;
    bool m_clicked = false;
    bool m_hovered = false;
};

