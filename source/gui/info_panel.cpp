#include "gui/info_panel.h"
#include "game_state.h"

InfoPanel::InfoPanel() {
    /*content = tgui::Group::create();
    content->setSize("100%", "100%");
    m_name = tgui::Label::create();
    m_name->getRenderer()->setTextColor(tgui::Color(255, 211, 3));
    content->add(m_name, "InfoPanelName");
    m_description = tgui::Label::create();
    m_description->setWidth("100%");
    m_description->setAutoSize(false); 
    m_description->setPosition(0, "InfoPanelName.bottom");
    m_description->getRenderer()->setTextColor(tgui::Color::White);
    content->add(m_description, "InfoPanelDescription");

    m_chars = tgui::Grid::create();
    m_chars->setAutoLayout(tgui::AutoLayout::Left);
    m_chars->setSize("100%", "height - InfoPanelDescription.bottom");
    m_chars->setPosition(0, "InfoPanelDescription.bottom");
    GameState::Instance().set_panel_content(content);*/
}

void InfoPanel::set_name(const std::string& name) {
    this->name = name;
    //m_name->setText(name);
}

void InfoPanel::set_description(const std::string& description) {
    this->description = description;
    //m_description->setText(description);
    //m_description->setHeight(m_description->getFullSize().y);
}

std::string to_string_2(double number) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << number;
    return out.str();
}

tgui::Widget::Ptr InfoPanel::create() {
    content = tgui::ScrollablePanel::create();
    content->setSize("100%", "100%");
    content->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Automatic);
    content->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
    content->getRenderer()->setBackgroundColor(sf::Color::Transparent);
    content->setContentSize({ 10000, 3000 });

    m_name = tgui::Label::create(name);
    m_name->getRenderer()->setTextColor(tgui::Color(255, 211, 3));
    content->add(m_name, "InfoPanelName");
    m_description = tgui::Label::create();
    m_description->onSizeChange.connect([d = m_description, c = content]() {
        d->setMaximumTextWidth(c->getSize().x);
    });
    m_description->setText(description);
    m_description->setPosition(0, "InfoPanelName.bottom");
    m_description->getRenderer()->setTextColor(tgui::Color::White);
    content->add(m_description, "InfoPanelDescription");

    m_chars = tgui::Grid::create();
    m_chars->setWidth("100%");
    m_chars->setHeight(40 * m_chars_saved.size());
    m_chars->setPosition(0, "InfoPanelDescription.bottom");
    int r = 0;
    for (auto& c : m_chars_saved) {
        auto name = tgui::Label::create(c.first);
        name->getRenderer()->setTextColor(sf::Color::White);
        name->onSizeChange.connect([n = name, c = m_chars]() {
            n->setMaximumTextWidth(c->getSize().x * 0.75);
        });
        auto p = tgui::Label::create(c.second);
        p->getRenderer()->setTextColor(sf::Color::White);
        p->setWidth("25%");
        p->onSizeChange.connect([p = p, c = m_chars, n = m_chars_saved.size()]() {
            p->setMaximumTextWidth(c->getSize().x * 0.25);
            p->setHeight(c->getSize().y / n);
        });
        m_chars->addWidget(name, r, 0, tgui::Grid::Alignment::Left, 1.);
        m_chars->addWidget(p, r, 1, tgui::Grid::Alignment::Left, 1.);
        ++r;
    }
    content->add(m_chars);
    return content;
}

void InfoPanel::add_char(const std::string& c, const std::string& p) {
    m_chars_saved.push_back(std::make_pair(c, p));
}

void InfoPanel::add_char(const std::string& c, float value) {
    add_char(c, to_string_2(value));
}

void InfoPanel::add_char(const std::string& c, int value) {
    add_char(c, std::to_string(value));
}

