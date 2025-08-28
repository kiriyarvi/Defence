#pragma once
#include <string>
#include <TGUI/TGUI.hpp>
#include <string>

class InfoPanel {
public:
    InfoPanel();

    void set_name(const std::string& name);
    void set_description(const std::string& description);
    void add_char(const std::string& c, float value);
    void add_char(const std::string& c, int value);
    void add_char(const std::string& c, const std::string& p);
    tgui::Widget::Ptr create();
    tgui::ScrollablePanel::Ptr content;
private:
    tgui::Label::Ptr m_name;
    tgui::Label::Ptr m_description;
    tgui::Grid::Ptr m_chars;
    int m_rows = 0;

    std::string name;
    std::string description;
    std::vector<std::pair<std::string, std::string>> m_chars_saved;

};
