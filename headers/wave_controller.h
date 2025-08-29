#pragma once
#include <enemies/IEnemy.h>

#include <map>
#include <vector>
#include <string>
#include "tile_map.h"

struct EnemySpawn {
    EnemyType type;
    double delay;
    bool boss = false;
};

struct IRoute {
    IRoute() = default;
    IRoute(IRoute&&) = default;
    virtual std::pair<EnemySpawn, bool> next_enemy() = 0;
    virtual std::string description() = 0;
    virtual ~IRoute() = default;
    RoadGraph::PathID id;
};

class UniformSpawner : public IRoute {
public:
    std::pair<EnemySpawn, bool> next_enemy() override;
    void add_spawner(EnemyType type, int count, bool boss = false);
    std::string description() override;
private:  
    struct Spawn {
        EnemyType type;
        int count;
        bool boss = false;
    };
    std::list<Spawn> m_spawners;
};

class ControlledSpawner : public IRoute {
public:
    void add_enemy(EnemyType type, float count, bool boss = false);
    std::pair<EnemySpawn, bool> next_enemy() override;
    std::string description() override;
private:
    int m_current_spawner = 0;
    std::vector<EnemySpawn> m_spawners;
};

struct Wave {
    std::vector<std::unique_ptr<IRoute>> routes;
    double prepairing_time = 10;
    int reward;
};

class WaveController {
public:
    WaveController();
    void logic(double dtime_microseconds);
    bool next_wave();
    void start_wave();
private:
    bool set_active_wave(int wave);
    std::vector<Wave> m_waves;
    int m_current_wave = 0;
    struct RouteState {
        RouteState(IRoute& spawner);
        float timer = 0;
        EnemySpawn current_enemy_spawn;
        IRoute& route;
        bool completed = false;
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
