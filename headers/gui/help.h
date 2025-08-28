#pragma once
#include <TGUI/TGUI.hpp>


class Tabs {
public:
    Tabs();
    void create_tab(const std::string& name, tgui::Widget::Ptr content);
    tgui::Group::Ptr get_content() { return m_content; }
private:
    tgui::Group::Ptr m_content;
    tgui::Group::Ptr m_tabs;
    tgui::Group::Ptr m_tab_content;
    std::vector<tgui::Button::Ptr> m_buttons;
};

class Help {
public:
    Help();
    tgui::Group::Ptr get_content() { return m_content; }
private:
    void show_guns_cat();
    void show_enemies_cat();
private:
    tgui::Panel::Ptr m_content;
    tgui::Group::Ptr m_cat_content;
    Tabs m_guns_tabs;
    Tabs m_enemies_tabs;

};
