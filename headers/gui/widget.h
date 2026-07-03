#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <list>
#include <type_traits>
#include <glm/vec2.hpp>
#include <memory>

class LayoutNode {
public:
    struct Spacing {
        float left = 0;
        float right = 0;
        float top = 0;
        float bottom = 0;
    };

    struct Rect {
        float x = 0; // относительные координаты
        float y = 0;
        float width = 0;
        float height = 0;
    };

    struct Layout {
        Rect layout_rect;
        //Параметры отступов не вычисляются автоматически и задаются пользователем.
        Spacing margin; //отступ от родительского виджета.
        Spacing padding; //отступ для контента.
        Rect get_content_rect() const;
        Rect get_border_rect() const; 
        glm::vec2 layout_size(const glm::vec2& content_size) const;
    } layout;

    struct Dependency {
        static int Size;
        static int Position;
        int type;
        LayoutNode* layout_node;
    };
    std::list<Dependency> size_dependencies;
    std::list<Dependency> position_dependencies;

    void add_dependent(int that, LayoutNode* depentent, int relation);

    std::function<glm::vec2()> position_function;
    std::function<glm::vec2()> size_function;
    void calc_position(uint32_t update_frame);
    void calc_size(uint32_t update_frame);

    //Layout functions
    void size_options_reset();
    void position_options_reset();
    //Simple
    void size_fixed(float width, float height);
    void size_inherited(LayoutNode* parent);
    void size_include(LayoutNode* child);
    void size_fraction(LayoutNode* parent, float parent_width_fraction, float parent_height_fraction);
    void position_centering(LayoutNode* parent);

    //Containers
    void vbox(const std::vector<LayoutNode*>& elements);
private:
    uint32_t m_last_size_update = 0;
    uint32_t m_last_position_update = 0;
};

class Widget: public LayoutNode {
public:
    Widget(Widget* parent = nullptr) : m_parent{ parent } {}
    static std::unique_ptr<Widget> create(Widget* parent = nullptr);
    void draw_hierarchy(int frame, const glm::vec2& position_transform, sf::RenderWindow& window);
    virtual void draw(const glm::vec2& position_transform, sf::RenderWindow& window) {}
    Widget* add(std::unique_ptr<Widget>&& child);
    Widget* m_parent;
    std::list<std::unique_ptr<Widget>> m_children;
};


class Panel : public Widget {
public:
    Panel(sf::Color background_color, sf::Color border_color, float border);
    static std::unique_ptr<Panel> create(
        sf::Color background_color = sf::Color(50, 50, 50, 200),
        sf::Color border_color = sf::Color::Black,
        float border = 3);
    void draw(const glm::vec2& position_transform, sf::RenderWindow& window) override;
private:
    sf::RectangleShape m_rect;
};
