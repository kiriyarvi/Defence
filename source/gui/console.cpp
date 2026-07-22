#include "gui/console.h"

void Console::add_message(const std::string& string) {
    auto label = Label::create(true);
    label->add_text(string);
    add_message(std::move(label));
}

void Console::add_message(std::unique_ptr<Label>&& label) {
    Message msg;
    msg.widget = label.get();
    msg.animation.set_duration(4);
    auto& fade = msg.animation.add_subanimation(3, 4, Animation());
    fade.on_progress = [l = label.get()](float p) {
        l->set_alpha(1. - p);
    };
    msg.animation.start();
    m_messages.push_back(std::move(msg));
    add_widget_smart(std::move(label));
    update_rules();
}

void Console::add_building_unlock_message(BuildingType type) {
    auto label = Label::create(true);
    label->add_text("Разблокирована постройка ");
    label->add_text(to_string(type), Label::unlock_buildng_color);
    label->add_text(".");
    add_message(std::move(label));
}

void Console::add_upgrade_unlock_message(Upgrade& upgrade, int level) {
    auto label = Label::create(true);
    label->add_text("Разблокирован апгрейд ");
    label->add_text(upgrade.get_name(level), Label::unlock_buildng_color);
    label->add_text(" для ");
    label->add_text(to_string(upgrade.building_type, LanguageCase::GENETIVE), Label::unlock_buildng_color);
    label->add_text(".");
    add_message(std::move(label));
}

void Console::logic(float dtime_mc) {
    bool erased = false;
    for (auto it = m_messages.begin(); it != m_messages.end();) {
        it->animation.logic(dtime_mc);
        if (it->animation.started())
            ++it;
        else {
            //не можем просто удалить it->widget потому что останется висячее правило vbox, поэтому сначала
            clear_rules(Property::SIZE);
            delete_widget(it->widget);
            it = m_messages.erase(it);
            erased = true;
        }
    }
    if (!erased)
        return;
    update_rules();
}


void Console::update_rules() {
    std::vector<Widget*> labels;
    for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it)
        labels.push_back(it->widget);
    vbox(labels);
}
