#pragma once
#include "gui/widget.h"

class ScaleSlider;

class Scale : public Widget {
public:
    Scale();
    Query on_event(Widget::EventContext event_context) override;
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
    size_t num_of_values = 4;
    bool clicked() { return m_clicked; }
    void set_on_pos_update_callback(const  std::function<void(size_t)>& callback);
private:
    ScaleSlider* m_slider;
    sf::Sprite m_tile_sprite;
    bool m_clicked = false;
    Widget* m_tooltip = nullptr;
};

class ScaleSlider : public Widget {
public:
    ScaleSlider(Scale* scale);
    Query on_event(Widget::EventContext event_context) override;
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
    void request_to_compute_pos() { m_query_to_compute_pos = true; }
    std::function<void(size_t)> on_pos_update;
    size_t get_pos() { return m_pos + 1; }
private:
    sf::Sprite m_slider;
    Scale* m_scale;
    size_t m_pos = 0;
    bool m_query_to_compute_pos = false;
};
