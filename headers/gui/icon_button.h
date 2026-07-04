#pragma once
#include "TGUI/TGUI.hpp"
#include "texture_manager.h"

#include "gui/widget.h"

class IconButton {
public:
    IconButton(TextureID icon, TextureID active_backgound, TextureID selected_background);
    enum class State {
        Locked,
        Active,
        Selected,
        Disabled,
    };
    State get_state() const { return m_state; }
    void set_state(State state);
public:
    tgui::Group::Ptr m_group;
    tgui::BitmapButton::Ptr m_button;
    tgui::Picture::Ptr m_lock;
private:
    void set_grayscale();
protected:
    State m_state;
    TextureID m_active_background;
    TextureID m_selected_background;
    TextureID m_icon;
};


class LayeredIcon: public Widget {
public:
    LayeredIcon() = default;
    std::vector<TextureID> layers;
    bool grayscale = false;
    void draw(const glm::vec2& position_transform, sf::RenderWindow& window) override;
protected:
    sf::Sprite m_sprite;
};
