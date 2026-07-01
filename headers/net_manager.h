#pragma once
#include <glm/vec2.hpp>
#include <list>

class Radar;

class NetManager {
public:
    // Получение единственного экземпляра
    static NetManager& Instance() {
        static NetManager instance; // Создаётся при первом вызове, потокобезопасно в C++11+
        return instance;
    }

    struct Net {
        struct CellID {
            int x_id;
            int y_id;
            bool operator==(const CellID&) const;
        };

        struct RadarInfo {
            CellID cell;
            Radar* radar_ptr;
            bool operator==(const RadarInfo& other) const;
        };

        std::list<RadarInfo> radars;
        std::list<CellID> radio_towers;

        bool in_uncover_zone(const glm::vec2& pos) const;
        bool in_radio_tower_zone(const glm::vec2& pos) const;
        void logic(float dtime_microseconds);
    };

    // Удаляем копирование и перемещение
    NetManager(const NetManager&) = delete;
    NetManager& operator=(const NetManager&) = delete;
    NetManager(NetManager&&) = delete;
    NetManager& operator=(NetManager&&) = delete;

    void new_radar(int x_id, int y_id, Radar* radar);
    void new_radio_tower(int x_id, int y_id);

    void logic(float dtime_microseconds);

    const Net& get_net_by_radiotower(Net::CellID id) const;

private:
    NetManager();
    std::list<Net> m_nets;
    std::list<Net::RadarInfo> m_radar_register;
};
