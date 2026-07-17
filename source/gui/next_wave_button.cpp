#include "gui/next_wave_button.h"
#include "gui/label.h"
#include "enemy_manager.h"

NextWaveButton::NextWaveButton() : Hoverable(this), Clickable(this, sf::Mouse::Left) {
    //inactive state
    layers = { TextureID::UpgradeButtonBackground, TextureID::NextWaveIcon };
    grayscale = true;

    on_hovered = [this](EventContext context) -> Query {
        //создаем tooltip
        auto [panel_ptr, label] = create_tooltip(Anchor::BOTTOM | Anchor::RIGHT);
        label->add_text("начать следующую волну");
        m_tooltip = panel_ptr.get();
        add_widget_deffered(std::move(panel_ptr));
        return Query{ Query::Workflow::PROCESSED };
    };
    on_unhovered = [this](EventContext context) ->size_t {
        delete_widget_deffered(m_tooltip, Widget::RemovePolicy::Min);
        return 0;
    };
    on_mouse_moved = [this](EventContext context) ->Query {
        m_tooltip->invalidate(Property::POSITION); //инвалидируем позицию у tooltip, чтобы он пересчитал её.
        return Query{ Query::Workflow::PROCESSED };
    };
    on_pressed = [this](EventContext context) ->Query {
        if (m_active) {
            layers = { TextureID::UpgradeButtonBackgroundCompleted, TextureID::NextWaveIcon };
            return Query{ Query::Workflow::PROCESSED };
        }
        else
            return Query{};
    };
    on_released = [this](EventContext context) ->Query {
        if (m_active) {
            EnemyManager::Instance().start_wave();
            layers = { TextureID::UpgradeButtonBackground, TextureID::NextWaveIcon };
            return Query{ Query::Workflow::PROCESSED };
        }
        else
            return Query{ Query::Workflow::PASS }; //PASS_TO_PARENT запрещен, поскольку это событие получаем по подписке.
    };
}

void NextWaveButton::set_active(bool active) {
    if (m_active != active) {
        if (active) {
            layers = { TextureID::UpgradeButtonBackground, TextureID::NextWaveIcon };
            grayscale = false;
        }
        else {
            layers = { TextureID::UpgradeButtonBackground, TextureID::NextWaveIcon };
            grayscale = true;
        }
    }
    m_active = active;
}

Query NextWaveButton::on_event(Widget::EventContext event_context) {
    Query q;
    q = hover_event(event_context);
    if (!q.pure_pass()) return q;
    q = click_event(event_context);
    return q;
}
