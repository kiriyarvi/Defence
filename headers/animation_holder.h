#pragma once
#include <SFML/Graphics.hpp>
#include <list>
#include <memory>


class IAnimationObject {
public:
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual bool logic(double dtime_mc) = 0; // true --- значит конец.
};

class AnimationHolder {
public:
    // Получение единственного экземпляра
    static AnimationHolder& Instance() {
        static AnimationHolder instance; // Создаётся при первом вызове, потокобезопасно в C++11+
        return instance;
    }

    // Удаляем копирование и перемещение
    AnimationHolder(const AnimationHolder&) = delete;
    AnimationHolder& operator=(const AnimationHolder&) = delete;
    AnimationHolder(AnimationHolder&&) = delete;
    AnimationHolder& operator=(AnimationHolder&&) = delete;
    void draw(sf::RenderWindow& window);
    void logic(double dtime_mc);
    void add_object(std::unique_ptr<IAnimationObject>&& object) { m_objects.push_back(std::move(object)); }
private:
    AnimationHolder() = default;
    std::list<std::unique_ptr<IAnimationObject>> m_objects;
};
