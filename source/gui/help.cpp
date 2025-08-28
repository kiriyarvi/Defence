#include "gui/help.h"
#include "gui/info_panel.h"
#include "params_manager.h"
#include "game_state.h"

Tabs::Tabs() {
    m_content = tgui::Group::create();
    m_content->setSize("100%", "100%");
    m_tabs = tgui::Group::create();
    m_tabs->setSize("20%", "100%");
    m_content->add(m_tabs, "Tabs");
    m_tab_content = tgui::Group::create();
    m_tab_content->setSize("80%", "100%");
    m_tab_content->setPosition("Tabs.right", 0);
    m_content->add(m_tab_content);

    m_tabs->onSizeChange([=]() {
        const float spacing = 4.f;
        float size = m_content->getSize().x * 0.1;
        float y = 0;
        for (auto& button : m_buttons) {
            float h = m_content->getSize().y * 0.05;
            //button->setSize({ m_content->getSize().x, h });
            button->setPosition({ 0, y });
            y += h + spacing;
        }
    });
}

void Tabs::create_tab(const std::string& name, tgui::Widget::Ptr content) {
    auto button = tgui::Button::create(name);
    button->setSize("100%", "5%");
    button->onClick.connect([=]() {
        m_tab_content->removeAllWidgets();
        m_tab_content->add(content);
    });
    m_tabs->add(button);
    m_buttons.push_back(button);
}

Help::Help() {
    m_content = tgui::Panel::create();
    m_content->getRenderer()->setBackgroundColor(sf::Color(50, 50, 50, 255));
    tgui::Group::Ptr categories = tgui::Group::create();
    categories->setSize("100%", "10%");
    m_content->add(categories, "Categories");
    m_cat_content = tgui::Group::create();
    m_cat_content->setSize("100%", "90%");
    m_cat_content->setPosition(0, "Categories.bottom");
    m_content->add(m_cat_content);

    tgui::Button::Ptr guns_button = tgui::Button::create("Оборона");
    categories->add(guns_button, "GunsButton");
    guns_button->onClick.connect([=]() {
        show_guns_cat();
    });
    tgui::Button::Ptr enemies_button = tgui::Button::create("Вражеские войска");
    enemies_button->setPosition("GunsButton.right", 0);
    categories->add(enemies_button);
    enemies_button->onClick.connect([=]() {
        show_enemies_cat();
    });

    tgui::Button::Ptr close_button = tgui::Button::create("Закрыть");
    close_button->setPosition("100%", 0);
    close_button->setOrigin(1.,0);
    categories->add(close_button);
    close_button->onClick.connect([=]() {
        GameState::Instance().display_help(false);
    });


    // GUNS TABS
    // Пулемет
    {
        auto& params = ParamsManager::Instance().params.guns.minigun;
        InfoPanel panel;
        panel.set_name("Пулемёт");
        panel.set_description(
            "Первоначальное, довольно капризное орудие. При стрельбе пулемет нагревается. "
            "Существует критическая температура. Время работы пулемета при нагреве выше критичесокой температуры ограничено. "
            "При превышении ограничения наступает перегрев: пулемет перестает стрелять до тех пор, пока не охладиться. "
            "Урон бронепробиваемость и скорострельность пулемета растут с повышением температуры."
        );
        panel.add_char("радиус поражения", params.radius);
        panel.add_char("урон при минимальном нагреве", params.min_damage);
        panel.add_char("урон при максимальном нагреве", params.max_damage);
        panel.add_char("бронепробиваемость при наименьшем нагреве", params.penetration_upgrades[0].min_armor_penetration_level);
        panel.add_char("бронепробиваемость при наибольшем нагреве", params.penetration_upgrades[0].max_armor_penetration_level);
        panel.add_char("частота выстрелов в секунду при минимальном нагреве", params.min_rotation_speed / 60.f);
        panel.add_char("частота выстрелов в секунду при максимальном нагреве", params.max_rotation_speed / 60.f);
        panel.add_char("время до максимального нагрева", params.heating_time);
        panel.add_char("время охлажения с максимлаьного нагрева до холодного состояния", params.cooling_time);
        panel.add_char("минимальная температура", 0);
        panel.add_char("максимальная температура", 1000);
        panel.add_char("значение критической температуры", params.critical_temperature * 1000);
        panel.add_char("время работы при критической температуре (до перегрева)", params.critical_temperature_work_duration);
        panel.add_char("время востановления после перегрева", params.cooldown_duration);

        m_guns_tabs.create_tab("Пулемёт", panel.create());
    }
    // Мина
    {
        auto& params = ParamsManager::Instance().params.guns.mine;
        InfoPanel panel;
        panel.set_name("Мина");
        panel.set_description(
            "При активации взрывается, нанося всем противникам урон в радиусе поражения. "
            "Чем дальше противник от эпицентра взрыва, тем меньший урон он получит. "
        );
        panel.add_char("радиус поражения", params.damage_radius);
        panel.add_char("урон в эпицентре", params.max_damage);
        panel.add_char("урон на радиусе", params.min_damage);
        panel.add_char("бронепробиваемость", params.armor_penetration_level);
        m_guns_tabs.create_tab("Мина", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.guns.antitank;
        InfoPanel panel;
        panel.set_name("Противотанковая пушка");
        panel.set_description(
            "Мощное орудие, имеющее высокий уровень бронепробиваемости и широкий радиус поражения."
        );
        panel.add_char("радиус поражения", params.radius);
        panel.add_char("урон", params.damage);
        panel.add_char("время перезарядки", params.cooldown);
        panel.add_char("бронепробиваемость", params.armor_penetration_level);
        m_guns_tabs.create_tab("Противотанковая пушка", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.guns.spikes;
        InfoPanel panel;
        panel.set_name("Шипы");
        panel.set_description(
            "Прокалывает колёса, в результате чего противник останавливается на некоторое время. "
            "Шипы теряют прочность каждый раз, когда останавливают противника. "
            "При достижении нулевой прочности шипы ломаются. "
            "Техника с тяжёлыми гусеницами моментально уничтожает шипы. "
            "Бесполезны против пехоты и гусеничной техники."
        );
        panel.add_char("длительность остановки противника", params.delay);
        panel.add_char("прочность", params.health);
        m_guns_tabs.create_tab("Шипы", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.guns.hedgehog;
        InfoPanel panel;
        panel.set_name("Противотанковые ежи");
        panel.set_description(
            "Останавливают крупную колёсную и гусеничную технику на некоторое время. "
            "Теряют прочность каждый раз, когда останавливают гусеничную технику. "
            "При достижении нулевой прочности ломаются. "
            "Колёсная техника урон не наносит. "
            "Техника с тяжёлыми гусеницами моментально уничтожает противотанковые ежи. "
            "Колёсная техника останавливается на большее время. "
            "Бесполезны против пехоты и мотоциклистов."
        );
        panel.add_char("длительность остановки гусеничной техники", params.delay);
        panel.add_char("длительность остановки колёсной техники", params.delay* params.wheels_debuff);
        panel.add_char("прочность", params.health);
        m_guns_tabs.create_tab("Противотанковые ежи", panel.create());
    }

    // ENEMIES
    {
        auto& params = ParamsManager::Instance().params.enemies.solder;
        InfoPanel panel;
        panel.set_name("Солдат");
        panel.set_description(
            "Не доставит вам серьезных проблем. Обычный пулемет без улучшений легко справиться с ним."
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("Солдат", panel.create());
        
    }
    {
        auto& params = ParamsManager::Instance().params.enemies.bike;
        InfoPanel panel;
        panel.set_name("Мотоциклист");
        panel.set_description(
            "Довольное неприятный противник из-за своей высокой скорости. Используйте несколько пулеметов, чтобы победить его. "
            " Победа над мотоциклистом откроет вам шипы, позволяющие останавливать колесную технику. "
            " После получения шипов мотоциклист перетанет быть для вас головной болью. "
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("Мотоциклист", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.enemies.pickup;
        InfoPanel panel;
        panel.set_name("Пикап");
        panel.set_description(
            "Простая легковая машина, ее бронирование ничего из себя не представляет, поэтому можно использовать обычный пулемет."
            " Победа над пикапом откроет улучшение для пулемета \"Бронебойные снаряды I\", увеличиваюющее бронепробиваемость."
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("Пикап", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.enemies.truck;
        InfoPanel panel;
        panel.set_name("Грузовик");
        panel.set_description(
            "Обычный военный грузовок, который везет взвод солдат к вам в тыл. Если остановите его, получите можное оружие - Мину."
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("Грузовик", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.enemies.BTR;
        InfoPanel panel;
        panel.set_name("БТР");
        panel.set_description(
            "Довольно серьезный противник. Имеет уровень бронирования выше, чем вся техника, встречающаяся вам до этого. "
            "Для победы над ним можно воспользоваться обычным племетом с улучшением \"бронебойные снаряды I\". Пулемет без улучшений "
            "сможет пробивать БТР лишь при достижени нагрева в 500 градусов. Также можно использовать мины. Не надейтесь использовать шипы, "
            " на БТР (как и на любую гусенечную технику) они не действуют. После победы над БТР вы откроете противотанковые ежи - хорошее дополнение к шипам."
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("БТР", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.enemies.tank;
        InfoPanel panel;
        panel.set_name("Танк");
        panel.set_description(
            "Имеет уровень бронирования выше, чем у БТР. Это значит, что даже пулемет с улучшением  \"бронебойные снаряды I\" будет пробивать его лишь при достижении нагрева в 500 градусов. "
            "После победы над танком ваши страдания с бронированной техникой наконец закончатся и вы получите в свое распоряжение противотанковую пушку."
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("Танк", panel.create());
    }
    {
        auto& params = ParamsManager::Instance().params.enemies.CruiserI;
        InfoPanel panel;
        panel.set_name("Наземный крейсер I");
        panel.set_description(
            "Первый по-настоящему серьезный противник. Имеет внушительный запас прочности и высокий уровень бронирования. Используйте противотанковые пушки для его уничтожения. "
            "Если наземный крейсер I сможет пройти через вашу оборону - вы проиграите не зависимо от того, сколько у вас осталось здоровья. "
            "Победа над наземным крейсером I откроет для вас мощную сдвоенную пушку."
        );
        panel.add_char("здоровье", params.health);
        panel.add_char("скорость", params.speed);
        panel.add_char("награда", params.reward);
        panel.add_char("уровень бронирования", params.armor_level);
        m_enemies_tabs.create_tab("Наземный крейсер I", panel.create());
    }

}

void Help::show_guns_cat() {
    m_cat_content->removeAllWidgets();
    m_cat_content->add(m_guns_tabs.get_content());
}

void Help::show_enemies_cat() {
    m_cat_content->removeAllWidgets();
    m_cat_content->add(m_enemies_tabs.get_content());
}
