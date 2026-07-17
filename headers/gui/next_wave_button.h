#pragma once
#include "gui/icon_button.h"

class NextWaveButton : public LayeredIcon, private Hoverable, private Clickable {
public:
    NextWaveButton();
    void set_active(bool active);
private:
    Query on_event(Widget::EventContext event_context) override;
    bool m_active = false;
    Widget* m_tooltip = nullptr;

};

