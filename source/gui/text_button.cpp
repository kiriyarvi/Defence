#include "gui/text_button.h"
#include "gui/label.h"
#include "texture_manager.h"
#include "shader_manager.h"

TextButton::TextButton(const std::string& text) {
    m_button_sprite.setTexture(TextureManager::Instance().textures[TextureID::TextButton]);
    Label* label = (Label*)add_widget(Label::create(true));
    DEBUG_TAG(label, "text_button_label");
    label->add_text(text);

    label->position_centering(this);

    add_rule(Property::WIDTH, [label, this](Layout& layout) {
        float button_size = std::round(((label->layout.width + 0.5 * layout.height) / layout.height) * 2) / 2.f;
        if (button_size == 3.5)
            button_size = 4;
        layout.width = button_size * layout.height;
        set_button_size(button_size);
    }, { {this, Property::HEIGHT }, {label, Property::WIDTH } });

   
}

Query TextButton::on_event(EventContext event_context) {
    if (event_context.event_type == Event::BUTTON_PRESSED && GUI::Instance().mouse_button == sf::Mouse::Left) { //кликнули левой кнопкой мыши
        if (m_enabled) { //кнопка активна => потенциальное взаимодействие
            GUI::Instance().subscribe_deffered(this, Event::BUTTON_RELEASED | Event::MOUSE_MOVED); //подпишемся
            m_clicked = true; //установим статус clicked
            set_button_size(m_button_size, true); //обновим background
        }
        //assert(m_hovered && "It is reachable??"); - да, вполне reachible в виду того, что может быть виджет поверх кнопки, блокирующий Event::MOUSE_MOVED
        return Query{ Query::PROCESSED };
    }
    else if (event_context.event_type == Event::BUTTON_RELEASED && GUI::Instance().mouse_button == sf::Mouse::Left) { //левую кнопку отпустили
        if (!event_context.from_subscribe)
            return Query{ Query::PROCESSED }; //не по подписке => не считается взаимодействием
        GUI::Instance().unsubscribe_deffered(this, Event::BUTTON_RELEASED | Event::MOUSE_MOVED); //отписываемся
        if (m_on_click && m_enabled) //если все еще m_enabled, выполним действие.
            m_on_click();
        m_clicked = false;
        set_button_size(m_button_size, true); //изменим background
        //проверим, нужно ли сделать unhover?
        if (std::find_if(event_context.hit_list.begin(), event_context.hit_list.end(), [this](const HitListNode& node) { return node.widget == this; }) == event_context.hit_list.end()) { //мышь не в hit-rect кнопки
            m_hovered = false;
            if (m_on_unhovered)
                m_on_unhovered();
        }
        return Query{ Query::PROCESSED };
    }
    else if (event_context.event_type == Event::MOUSE_MOVED) { //двинули мышь
        if (!event_context.from_subscribe) { //не по подписке
            GUI::Instance().subscribe_deffered(this, Event::MOUSE_MOVED); //подпишемся
            if (m_on_hovered) //hovered
                m_on_hovered();
            m_hovered = true;
        }
        else { //по подписке
            if (m_clicked) //если мышь зажата, то считаем что всегда hover, не зависимо ни от чего
                return Query{ Query::PROCESSED };
            //иначе проверим hit-test
            if (std::find_if(event_context.hit_list.begin(), event_context.hit_list.end(), [this](const HitListNode& node) { return node.widget == this; }) == event_context.hit_list.end()) { //мышь не в hit-rect кнопки
                //отписываемся
                GUI::Instance().unsubscribe_deffered(this, Event::MOUSE_MOVED);
                assert(m_hovered && "It is reachable??");
                m_hovered = false;
                if (m_on_unhovered)
                    m_on_unhovered();
            }

        }
        return Query{ Query::PROCESSED };
    }
    return Query::skip(event_context.from_subscribe);

}

void TextButton::set_button_size(float button_size, bool forced) {
    if (m_button_size == button_size && !forced)
        return;
    int offset = m_clicked ? 64 : 0;
    m_button_size = button_size;
    if (m_button_size == 1)
        m_button_sprite.setTextureRect(sf::IntRect(48 + offset, 16, 16, 16));
    else if (m_button_size == 1.5)
        m_button_sprite.setTextureRect(sf::IntRect(32 + offset, 0, 24, 16));
    else if (m_button_size == 2)
        m_button_sprite.setTextureRect(sf::IntRect(0 + offset, 0, 32, 16));
    else if (m_button_size == 2.5)
        m_button_sprite.setTextureRect(sf::IntRect(0 + offset, 48, 40, 16));
    else if (m_button_size == 3)
        m_button_sprite.setTextureRect(sf::IntRect(0 + offset, 16, 48, 16));
    else if (m_button_size >= 4)
        m_button_sprite.setTextureRect(sf::IntRect(0 + offset, 32, 64, 16));
}

void TextButton::draw(const glm::vec2& position_transform, sf::RenderTarget& window) {
    m_button_sprite.setPosition(position_transform.x + layout.x, position_transform.y + layout.y);
    m_button_sprite.setScale(layout.width / m_button_sprite.getTextureRect().width, layout.height / m_button_sprite.getTextureRect().height);
    sf::RenderStates states;
    if (!m_enabled)
        states.shader = &ShaderManager::Instance().shaders[Shader::GrayScale];
    window.draw(m_button_sprite, states);
}
