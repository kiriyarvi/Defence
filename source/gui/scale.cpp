#include "gui/scale.h"
#include "texture_manager.h"
#include "gui/label.h"

Scale::Scale() {
    m_tile_sprite.setTexture(TextureManager::Instance().textures[TextureID::Slider]);

    add_rule(Property::WIDTH, [this](Layout& layout) {
        layout.width = layout.height * (num_of_values - 1);
    }, { {this, Property::HEIGHT} });

    m_slider = (ScaleSlider*)add_widget(std::make_unique<ScaleSlider>(this));
}

Query Scale::on_event(Widget::EventContext event_context) {
    if (event_context.event_type == Event::BUTTON_PRESSED && GUI::Instance().mouse_button == sf::Mouse::Left) {
        if (m_tooltip) { //если есть tooltip => удалим чтоб не мешал
            m_parent->delete_widget_deffered(m_tooltip, RemovePolicy::Min);
            m_tooltip = nullptr;
        }
        m_clicked = true;
        m_slider->invalidate(Property::POSITION); //заставим пересчитать позицию.
        GUI::Instance().subscribe_deffered(this, Event::MOUSE_MOVED | Event::BUTTON_RELEASED); //подписываемся на получаение событий
        return Query{ Query::PROCESSED };
    }
    else if (event_context.event_type == Event::MOUSE_MOVED) {
        if (!event_context.from_subscribe) { //не по подписке, то есть просто навели мышь на шкалу.
            //создаем tooltip
            if (m_tooltip)
                return Query{ Query::PROCESSED };
            auto [panel_ptr, label] = create_tooltip(Anchor::BOTTOM | Anchor::RIGHT);
            label->add_text("Ускорение: x" + std::to_string(m_slider->get_pos()));
            m_tooltip = panel_ptr.get();
            m_parent->add_widget_deffered(std::move(panel_ptr));
            GUI::Instance().subscribe_deffered(this, Event::MOUSE_MOVED); //подписка на перемещение мыши
            return Query{ Query::PROCESSED };
        }
        else {
            if (clicked()) { //перемещение мыши во время зажатой правой кнопки мыши
                m_slider->invalidate(Property::POSITION); //заставим пересчитать позицию.
                return Query{ Query::PROCESSED };
            }
            else { //кнопка не зажата, но получили по подписке => unhover
                if (event_context.hit_list.empty() || event_context.hit_list.back().widget != this) {
                    m_parent->delete_widget_deffered(m_tooltip, RemovePolicy::Min);
                    m_tooltip = nullptr;
                    GUI::Instance().unsubscribe_deffered(this, Event::MOUSE_MOVED); //отменим подписку
                    return Query{ Query::REPEAT, Query::PERFORM_DEFFERED };
                }
                m_tooltip->invalidate(Property::POSITION);
                return Query{ Query::PROCESSED };
            }
        }
    }
    else if (event_context.event_type == Event::BUTTON_RELEASED && clicked()) {
        //отпишемся
        GUI::Instance().unsubscribe_deffered(this, Event::MOUSE_MOVED | Event::BUTTON_RELEASED);
        m_clicked = false;
        m_slider->request_to_compute_pos();// заставим вычислить новое деление, у которого теперь будет расположен slider.
        m_slider->invalidate(Property::POSITION); //заставим пересчитать позицию.
        return Query{ Query::PROCESSED };
    }
    else if (event_context.from_subscribe)
        return Query{ Query::PROCESSED };
    else
        return Query{ Query::PASS_TO_PARENT };
}

void Scale::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    m_tile_sprite.setScale({ layout.height / 16.f, layout.height / 16.f });
    m_tile_sprite.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    for (size_t i = 1; i < num_of_values; ++i) {
        if (i == 1)
            m_tile_sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));
        else if (i == num_of_values - 1)
            m_tile_sprite.setTextureRect(sf::IntRect(32, 0, 16, 16));
        else
            m_tile_sprite.setTextureRect(sf::IntRect(16, 0, 16, 16));
        window.draw(m_tile_sprite);
        m_tile_sprite.move({ layout.height, 0 });
    }

}

void Scale::set_on_pos_update_callback(const  std::function<void(size_t)>& callback) {
    m_slider->on_pos_update = callback;
}


ScaleSlider::ScaleSlider(Scale* scale): m_scale{scale} {
    m_slider.setTexture(TextureManager::Instance().textures[TextureID::Slider]);
    m_slider.setTextureRect(sf::IntRect(48, 1, 5, 14));

    add_rule(Property::SIZE, [this](Layout& layout) {
        layout.height = m_scale->layout.height * (14.f/16.f);
        layout.width =  m_scale->layout.height * (5.f/16.f);
    }, { {m_scale, Property::HEIGHT } });

    add_rule(Property::POSITION, [this](Layout& layout) {
        if (m_query_to_compute_pos) {
            Scale* scale = static_cast<Scale*>(m_parent);
            auto content_transform = m_parent->get_content_transform();
            auto mouse_pos = GUI::Instance().mouse_pos;
            float x = mouse_pos.x - content_transform.x;
            float max = m_parent->layout.width - 2.f * m_parent->layout.height / 16.f;
            x = std::clamp<float>(x, 0, max);
            size_t new_pos = std::round(x / max * (scale->num_of_values - 1));
            if (new_pos != m_pos) {
                if (on_pos_update)
                    on_pos_update(new_pos);
                m_pos = new_pos;
            }
            m_query_to_compute_pos = false;
        }
        if (m_scale->clicked()) {
            auto content_transform = m_parent->get_content_transform();
            auto mouse_pos = GUI::Instance().mouse_pos;
            float x = mouse_pos.x - content_transform.x;    
            x = std::clamp<float>(x, 0, m_parent->layout.width - 2.f * m_parent->layout.height / 16.f); // slider может доходить только до половины крайних делений, поэтому вычитаем небольшие отступы по краям,
            layout.x = x - layout.width / 2.f + m_scale->layout.height / 16.f;
            layout.y = (m_scale->layout.height - layout.height) / 2.f;
        }
        else {
            float additional_offset = 0.0;
            if (m_pos == 0) {
                additional_offset = m_scale->layout.height / 16.f;
            }
            else if (m_pos == m_scale->num_of_values - 1) {
                additional_offset = -m_scale->layout.height / 16.f;
            }
            layout.x = m_scale->layout.height * m_pos - layout.width / 2.f + additional_offset; //начало клетки, выравнивание по центру Slider, последняя добавка - выравнивание по центру деления шкалы
            layout.y = (m_scale->layout.height - layout.height) / 2.f;
        }

    }, { { m_scale, Property::LAYOUT }, { this, Property::SIZE } });
}

Query ScaleSlider::on_event(Widget::EventContext event_context) {
    return Query{};
}

void ScaleSlider::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    m_slider.setScale({ layout.width / 5.f, layout.height / 14.f });
    m_slider.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    window.draw(m_slider);
}
