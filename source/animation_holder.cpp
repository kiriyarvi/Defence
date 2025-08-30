#include "animation_holder.h"
#include <algorithm>

void AnimationHolder::draw(sf::RenderWindow& window) {
    for (auto& obj : m_objects)
        obj->draw(window);
}

void AnimationHolder::logic(double dtime_mc) {
    auto it = m_objects.begin();
    while (it != m_objects.end()) {
        if ((*it)->logic(dtime_mc))
            it = m_objects.erase(it);
        else
            ++it;
    }
}
