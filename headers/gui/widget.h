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

class GUI {
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
    void event(const sf::Event& event);
    struct StackElement {
        Widget* node;
        size_t properties;
        bool operator==(const StackElement& element) { return node == element.node && properties == element.properties; }
    };
    std::list<StackElement> layout_stack;

    void invalidate_event_states();
    glm::vec2 mouse_pos;
    glm::vec2 window_size;
private:
    GUI() = default;
    std::unique_ptr<Widget> m_root;
    FrameID m_frame = 0;
    std::pair<Widget*, glm::vec2> m_hovered = { nullptr, glm::vec2{} };
    bool m_invalidated = false;
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
    //LAYOUT FUNCTIONS
    //SIZE
    using Modifier = std::function<float(float)>;
    void property_inherit(Widget* widget, Property::Type properties, const Modifier& modifier = {}); 
    void property_include(Widget* widget, Property::Type properties, const Modifier& modifier = {});
    void size_fixed(float width, float height);
    void size_inherited(Widget* widget);
    void size_include(Widget* widget);
    void size_fraction(Widget* widget, float parent_width_fraction, float parent_height_fraction);
    //POSITION
    void position_centering(Widget* parent = nullptr);
    void position_tooltip(size_t ancher);
    void position_anchor(Anchor::Type pivot, Widget* to, Anchor::Type anchor);
    //CONTAINERS
    void vbox(const std::vector<Widget*>& elements);
    //WIDGET HIERARCHY
    Widget* add(std::unique_ptr<Widget>&& child);
    void delete_widget(Widget* widget); // NOTE: не удаляет автоматически layout rules виджетов, которые сслыаются на widget.
    void draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderWindow& window);
    virtual void draw(const glm::vec2& position_transform, sf::RenderWindow& window) {}
    //EVENT SYSTEM
    std::function<void(const glm::vec2& position_transform, const glm::vec2& mouse_pos)> on_hovered;
    std::function<void(const glm::vec2& position_transform, const glm::vec2& mouse_pos)> on_mouse_moved;
    std::function<void(const glm::vec2& position_transform, const glm::vec2& mouse_pos)> on_unhovered;
    std::function<void(const glm::vec2& position_transform, const glm::vec2& mouse_pos, const sf::Event::MouseButtonEvent& event)> on_pressed;
    std::function<void(const glm::vec2& position_transform, const glm::vec2& mouse_pos, const sf::Event::MouseButtonEvent& event)> on_released;

    std::pair<Widget*, glm::vec2> get_widget_under_cursor(const glm::vec2& parent_transform, glm::uvec2 mouse_pos);
    bool receive_mouse_events = true;

    virtual ~Widget();
#ifdef GUI_DEBUG_ENABLED
    std::string debug_name;
#endif
protected:
    Widget* m_parent;
    std::list<std::unique_ptr<Widget>> m_children;
    std::list<Rule> m_rules;
};


class Panel : public Widget {
public:
    Panel(sf::Color background_color, sf::Color border_color, float border);
    static std::unique_ptr<Panel> create(
        sf::Color background_color = sf::Color(50, 50, 50, 200),
        sf::Color border_color = sf::Color::Black,
        float border = 3);
    void set_border(float border);
    void draw(const glm::vec2& position_transform, sf::RenderWindow& window) override;
private:
    sf::RectangleShape m_rect;
};
