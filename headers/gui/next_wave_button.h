#pragma once
#include "gui/layered_icon.h"

class NextWaveButton : public LayeredIcon, private HoverableClickableWidget {
public:
    NextWaveButton();
    void set_active(bool active);
private:
    bool m_active = false;
    Widget* m_tooltip = nullptr;

};

