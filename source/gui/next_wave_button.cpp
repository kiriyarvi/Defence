#include "gui/next_wave_button.h"
#include "gui/label.h"
#include "gui/tooltip.h"
#include "enemy_manager.h"

NextWaveButton::NextWaveButton() {
    //inactive state
    layers = { TextureID::ButtonBackground, TextureID::NextWaveIcon };
    grayscale = true;
    m_button = Button::LEFT;

    m_on_hovered = [this]()  {
        //создаем tooltip
        auto [panel_ptr, label] = create_tooltip(Anchor::BOTTOM | Anchor::RIGHT);
        label->add_text("начать следующую волну");
        m_tooltip = panel_ptr.get();
        m_parent->add_widget_deffered(std::move(panel_ptr));
    };
    m_on_unhovered = [this]()  {
        m_parent->delete_widget_deffered(m_tooltip);
    };
    m_on_mouse_moved = [this]()  {
        m_tooltip->invalidate(Property::POSITION); //инвалидируем позицию у tooltip, чтобы он пересчитал её.
    };
    m_on_pressed = [this](Button::Type) {
        if (m_active) {
            layers = { TextureID::ButtonClickedBackground, TextureID::NextWaveIcon };
        }
    };
    m_on_released = [this](Button::Type) {
        if (m_active) {
            EnemyManager::Instance().start_wave();
            layers = { TextureID::ButtonBackground, TextureID::NextWaveIcon };
        }
    };
}

void NextWaveButton::set_active(bool active) {
    if (m_active != active) {
        if (active) {
            layers = { TextureID::ButtonBackground, TextureID::NextWaveIcon };
            grayscale = false;
        }
        else {
            layers = { TextureID::ButtonBackground, TextureID::NextWaveIcon };
            grayscale = true;
        }
    }
    m_active = active;
}
