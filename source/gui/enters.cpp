#include "gui/enters.h"
#include "gui/tooltip.h"
#include "enemy_manager.h"

void EntersWidget::create_tooltip(const std::string& content) {
    if (m_tooltip_widget)
        delete_tooltip();
    auto [panel, label] =  ::create_tooltip(Anchor::LEFT | Anchor::BOTTOM);
    label->add_text(content);
    m_tooltip_widget = panel.get();
    m_ui->add_widget(std::move(panel));
}

void EntersWidget::delete_tooltip() {
    m_ui->delete_widget(m_tooltip_widget);
    m_tooltip_widget = nullptr;
}

void EntersWidget::on_event(const sf::RenderWindow& window, sf::Event event) {
    if (event.type != sf::Event::MouseMoved)
        return;
    sf::Vector2i mouse_screen_pos(event.mouseMove.x, event.mouseMove.y);
    sf::Vector2f mouse_pos = window.mapPixelToCoords(mouse_screen_pos);
    sf::Vector2i cell_id(std::ceil(mouse_pos.x / 32.f), std::floor(mouse_pos.y / 32.f));
    bool mouse_on_enter = false;

    for (auto& enter : m_enters) {
        if (enter.y_id == cell_id.y && enter.x_id == cell_id.x) { // показать информацию о волне
            if (m_hovered_enter != &enter) {
                create_tooltip(enter.content);
                m_hovered_enter = &enter;
            }
            mouse_on_enter = true;
            break;
        }
    }
    if (mouse_on_enter)
        m_tooltip_widget->invalidate(Property::POSITION);
    
    if (!mouse_on_enter && m_tooltip_widget) {
        delete_tooltip();
        m_hovered_enter = nullptr;
    }
}

void EntersWidget::add_enter(RoadGraph::PathID id, const std::string& content) {
    auto& path = EnemyManager::Instance().all_paths[id.start_node][id.path];
    Enter enter{ path[0]->x, path[0]->y, id, content, RouteDrawer(path) };
    m_enters.push_back(enter);
}

void EntersWidget::delete_all_enters() {
    m_enters.clear();
    m_hovered_enter = nullptr;
}

EntersWidget::EntersWidget(Widget* ui) : m_ui{ui} {
    m_enter_sprite.setTexture(TextureManager::Instance().textures[TextureID::Arrow]);
}

void EntersWidget::draw(sf::RenderWindow& window) {
    for (auto& enter : m_enters) {
        m_enter_sprite.setPosition((enter.x_id - 1) * 32, enter.y_id * 32);
        window.draw(m_enter_sprite);
    }
    if (m_hovered_enter)
        m_hovered_enter->drawer.draw(window);
}


void EntersWidget::logic(float dtime_mc) {
    if (m_hovered_enter) {
        m_hovered_enter->drawer.logic(dtime_mc);
    }
}
