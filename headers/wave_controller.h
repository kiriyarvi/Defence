#pragma once
#include <enemies/IEnemy.h>

#include <unordered_map>
#include <vector>

class WaveController {
public:
    struct EnemySpawn {
        EnemyType type;
        double delay;
    };

    struct Route {
        int route;
        std::vector<EnemySpawn> spawner;
    };

    struct Wave {
        std::vector<Route> routes;
        double prepairing_time;
        int reward;
    };

    WaveController();
    void logic(double dtime_microseconds);
    bool next_wave();
private:
    bool set_active_wave(int wave);
    std::vector<Wave> m_waves;
    int m_current_wave = 0;
    struct RouteState {
        int current_spawner = 0;
        float timer = 0;
        Route& route;
        bool logic(double dtime_ms);
    };
    std::vector<RouteState> m_routes_states;
    double m_timer = 0;
    enum class State {
        Prepairing,
        Spawn,
        Completed
    } m_state = State::Prepairing;
};
