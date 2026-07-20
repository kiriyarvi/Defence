#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <list>
#include <type_traits>
#include <glm/vec2.hpp>
#include <memory>
#include <stack>
#include <unordered_map>

#ifdef GUI_DEBUG_ENABLED
    #define DEBUG_TAG(widget, tag) widget->debug_name = tag;
#else
    #define DEBUG_TAG(widget, tag)
#endif

class Widget;

namespace Event {
    using Type = size_t;
    inline Type MOUSE_MOVED = 0b0001;
    inline Type BUTTON_PRESSED = 0b0010;
    inline Type BUTTON_RELEASED = 0b0100;
    inline Type WHEEL_SCROLLED = 0b1000;
}


struct Query {
    enum Workflow {
        PROCESSED, //< событие обработано, завершить обработку события
        PASS, //< передать событие следующему виджету
        REPEAT, //< запустить обработку события заново
        PASS_TO_PARENT //< отправить родителю
    } workflow = Workflow::PASS_TO_PARENT; //< флаг управления потоком обработки события
    size_t query = 0; //< запросы, битовая маска, состоящая из:
    static const size_t PERFORM_DEFFERED = 0b01; //< выполнить все отложенные запросы (удаление/добавление виджетов/подписок), при этом workflow обязательно должен быть =REPEAT.
    static const size_t CALC_LAYOUT = 0b10; //< пересчитать layout
    bool pure_pass() { return workflow == Workflow::PASS && query == 0; }
    static Query skip(bool from_subscribe);
};

class WidgetIterator;

class GUI {
    friend class WidgetIterator;
public:
    static GUI& Instance() {
        static GUI instance;
        return instance;
    }
    // Удаляем копирование и перемещение
    GUI(const GUI&) = delete;
    GUI& operator=(const GUI&) = delete;
    GUI(GUI&&) = delete;
    GUI& operator=(GUI&&) = delete;

    void set_root(std::unique_ptr<Widget>&& root, const sf::RenderWindow& window);
    Widget* get_root() { return m_root.get(); }
    using FrameID = size_t;
    FrameID get_current_frame_id() { return m_frame;  }

    void draw(sf::RenderWindow& window);
    //EVENT_SYSTEM
    bool event(const sf::Event& event);
    void subscribe_deffered(Widget* widget, Event::Type type);
    void unsubscribe_deffered(Widget* widget, Event::Type type);
    bool is_event_processing() const { return m_event_processing; }
    void add_deffered_command(std::function<void()>&& command) { m_deffered_commands.push_back(std::move(command)); }
    /// переменные контекста. Сюда заносится информация о событии
    glm::vec2 mouse_pos;
    glm::vec2 window_size;
    sf::Mouse::Button mouse_button;
    Event::Type event_type;
    float wheel_delta;
#ifdef GUI_USER_CONTRACT_CHECKS_ENABLED
    struct StackElement {
        Widget* node;
        size_t properties;
        bool operator==(const StackElement& element) { return node == element.node && properties == element.properties; }
    };
    std::list<StackElement> layout_stack;
#endif
private:
    GUI() = default;
    bool event_impl();
    void perform_deffered();
    std::unique_ptr<Widget> m_root;
    FrameID m_frame = 0;

    struct Subscriber {
        Event::Type event_type;
        Widget* subscriber;
    };
    std::list<Subscriber> m_subscribers;

    std::list<std::function<void()>> m_deffered_commands;
    bool m_event_processing = false;
};


class Widget;

class Property {
    friend class Widget;
public:
    Property(float initial) : m_value{ initial } {}
    Property& operator=(float value) { m_value = value; return *this; }
    operator float() const { return m_value; }
    float get_value() const { return m_value; }

    GUI::FrameID last_calculation_frame_id = 0;

    using Type = size_t;
    static Type X;
    static Type Y;
    static Type WIDTH;
    static Type HEIGHT;
    static Type SIZE;
    static Type POSITION;
    static Type LAYOUT;
private:
    float m_value;

    struct Dependent {
        Widget* widget; //зависимый виджет
        Property::Type output; //поля, расчитываемые widget на основе данного Property.
    };
    void add_depentent(Dependent dependent); //< вспомогательная функция для добавления зависимого виджета.
    void clear_dependent(Dependent dependent); //< вспомогательная функция, которая вызывается, когда dependent.widget больше не зависит от данного layout.
    std::list<Dependent> dependents;
};

struct Anchor {
    using Type = size_t;
    static Type LEFT;
    static Type RIGHT;
    static Type TOP;
    static Type BOTTOM;
    static Type CENTER;
};

struct Spacing {
    float left = 0;
    float right = 0;
    float top = 0;
    float bottom = 0;
};

struct Rect {
    float x = 0.f;
    float y = 0.f;
    float width = 0.f;
    float height = 0.f;
};

using Modifier = std::function<float(float)>;

namespace modifiers {
    struct Multiply {
        Multiply(float m) : m{ m } {}
        float operator()(float v) { return v * m; }
        float m;
    };

    struct Add {
        Add(float a) : a{ a } {}
        float operator()(float v) { return v + a; }
        float a;
    };
}


class Widget {
public:
    Widget(Widget* parent = nullptr) : m_parent{ parent } {}
    static std::unique_ptr<Widget> create(Widget* parent = nullptr);

    struct Dependency {
        Widget* widget;
        Property::Type source; //свойства widget, которые подаются на вход правилу
    };

    //LAYOUT SYSTEM
    ///Каждый виджет ассоциирован с прямоугольнком, называемым layout
    ///его задают четыре свойства x,y,width, height.
    ///эти свойства вычисляются динамически с помощью "правил"
    ///пользователь может задать правила вычисления свойств с помощью
    ///метода add_rule.
    class Layout {
        friend class Widget;
    public:
        /// Вычисляемые динамически свойства layout
        Property x = 0.f;
        Property y = 0.f;
        Property width = 0.f;
        Property height = 0.f;
        /// Невычисляемые динамически свойства (параметры)
        Spacing padding; //отступ для контента.
        bool absolute = false; //по умолчанию координаты x,y указываются относительно родительского виджета.
    public:
        Rect get_content_rect() const;
        Rect get_layout_rect() const;
        glm::vec2 layout_size(const glm::vec2& content_size) const;
        glm::vec2 center() const;
        bool contains(const glm::vec2& transform, float x, float y);
        glm::vec2 get_anchor_relative_to_center(Anchor::Type anchor) const;
        Property::Type invalidated_props() const { return m_invalidated_props; }
    private:
        const static std::unordered_map<Property::Type, Property Layout::*> s_property_map;
        const static std::unordered_map<Property::Type, float Rect::*> s_property_map_rect;
        void invalidate(Property::Type props) { m_invalidated_props |= props; }
        Property::Type m_invalidated_props = Property::LAYOUT; //< в начале инвалидированы все свойства.
    } layout;

    struct Rule {
        Property::Type output; //OR-d by |
        std::function<void(Layout&)> calc_function;
        std::vector<Dependency> dependencies;
    };

    void add_rule(Property::Type output, const std::function<void(Layout&)>& calc, const std::vector<Dependency>& dependencies);
    void calc_properties(Property::Type property);
    void calc_layout();
    void invalidate(Property::Type property);
    void clear_rules(Property::Type properties);
    glm::vec2 get_position_transform() const;
    glm::vec2 get_content_transform() const;
    //LAYOUT FUNCTIONS
    //GENERAL
    void property_equal(Property::Type output, bool content_output, Widget* source, Property::Type input, bool content_input, const Modifier& modifier);   
    void property_from_content(Widget* source, Property::Type properties, const Modifier& modifier = {}); 
    void property_content_from(Widget* source, Property::Type properties, const Modifier& modifier = {});
    //SIZE
    void size_fixed(float width, float height);
    void size_inherited(Widget* source);
    void size_include(Widget* source);
    void size_fraction(Widget* source, float parent_width_fraction, float parent_height_fraction);
    //POSITION
    void position_centering(Widget* parent = nullptr);
    void x_centering(Widget* parent = nullptr);
    void y_centering(Widget* parent = nullptr);
    void position_tooltip(size_t ancher);
    void position_anchor(Anchor::Type pivot, Widget* to, Anchor::Type anchor);
    //CONTAINERS
    void vbox(const std::vector<Widget*>& elements);
    void hbox(const std::vector<Widget*>& elements);
    //WIDGET HIERARCHY
    enum class RemovePolicy {
        /// Минимальное удаление, сохраняющее работоспособность системы
        /// Инвалидирует Layout, указывает зависимостям, что данный виджет больше не зависит от них
        /// Не удаляем правила других виджетов, ссылающиеся на данный виджет.
        Min = 0b0001,
        /// Тоже что Min, но дополнительно удаляет правила других виджетов
        /// которые зависят от свойств данного виджета. Если обнаружено правило, которое зависит от данного
        /// виджета только частично, то выдает исключение.
        DeleteDepententRules = 0b0011, //< удалит правила других виджетов, зависимые только от свойств удаляемого виджета, если есть "смешанные правила" - исключение
        /// Тоже что Min, но дополнительно удаляет правила других виджетов
        /// которые зависят от свойств данного виджета. Если обнаружено правило, которое зависит от данного
        /// виджета только частично, оно всё равно удаляется.
        DeleteDepententRulesHard = 0b0101, //< удалит правила других виджетов, зависимые от свойств удаляемого виджета. Удаляет даже смешанные правила.
    };

    Widget* add_widget(std::unique_ptr<Widget>&& child);
    Widget* add_widget_deffered(std::unique_ptr<Widget>&& child);
    Widget* add_widget_smart(std::unique_ptr<Widget>&& child);
    void delete_widget(Widget* widget, RemovePolicy policy);
    void delete_widget_deffered(Widget* widget, RemovePolicy policy);
    void delete_widget_smart(Widget* widget, RemovePolicy policy);
    void delete_all_widgets(RemovePolicy policy);
    Widget* get_root();
    //DRAW CALLS
    virtual void draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderTarget& window);
    virtual void draw(const glm::vec2& position_transform, sf::RenderTarget& window) {}
    //EVENT SYSTEM
    struct HitListNode {
        Widget* widget;
        glm::vec2 parent_transform;
    };
    enum class HitTestPolicy {
        /// сначала проверяет hit_test детей, выполнение завершается как только для ребенка
        /// hit_test вернет true. В этом случае в hit_list добавляется путь до этого ребенка.
        /// в противном проверяет hit_test текущего виджета, если true, то в hit_list добавится текущий виджет.
        /// в противном случае hit_list не меняется и возвращается false
        Normal,
        // Сначала проверит hit_test текущего виджета. Если вернется false, то сразу завершает hit_test и возвращает false.
        // В противном случае реализуется та же логика, что при Normal
        Block,
        //Сразу вернет false
        Terminate
    } hit_test_policy = HitTestPolicy::Normal;

    bool hit_test(std::list<HitListNode>& hit_list, glm::uvec2 mouse_pos);
    struct EventContext {
        const std::list<HitListNode>& hit_list;
        Event::Type event_type;
        bool from_subscribe;
    };
    virtual Query on_event(EventContext event_context) { return Query{}; }

    virtual ~Widget() = default;
#ifdef GUI_DEBUG_ENABLED
    std::string debug_name;
#endif
protected:
    Widget* m_parent;
    std::list<std::unique_ptr<Widget>> m_children;
    std::list<Rule> m_rules;
    void remove(RemovePolicy policy); //< только перестаивает дерево соотвествующим образом, не совершает реального удаления, не удаляет детей
    void delete_dependent_rules(Property::Type props, bool hard);
};


class Panel : public Widget {
public:
    Panel(sf::Color background_color, sf::Color border_color, float border);
    static std::unique_ptr<Panel> create(
        sf::Color background_color = sf::Color(50, 50, 50, 200),
        sf::Color border_color = sf::Color::Black,
        float border = 3);
    void set_border(float border);
    void draw(const glm::vec2& position_transform, sf::RenderTarget& window) override;
private:
    sf::RectangleShape m_rect;
};

class Hoverable {
public:
    Hoverable(Widget* widget) : m_widget{ widget } {}
    Query hover_event(Widget::EventContext event_context);
    std::function<Query(Widget::EventContext)> on_hovered;
    std::function<size_t(Widget::EventContext)> on_unhovered;
    std::function<Query(Widget::EventContext)> on_mouse_moved;
private:
    Widget* m_widget;
};

class Clickable {
public:
    Clickable(Widget* widget, sf::Mouse::Button button) : m_widget{ widget }, m_button{ button } {}
    Query click_event(Widget::EventContext event_context);
    std::function<Query(Widget::EventContext)> on_pressed;
    std::function<Query(Widget::EventContext)> on_released;
    bool clicked() const { return m_clicked; }
private:
    bool m_clicked = false;
    Widget* m_widget;
    sf::Mouse::Button m_button;
};
