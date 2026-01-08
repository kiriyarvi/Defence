#include "wave_controller.h"
#include "enemy_manager.h"
#include "game_state.h"
#include <unordered_map>
#include <random>

void UniformSpawner::add_spawner(EnemyType type, int count, bool boss) {
    m_spawners.push_back(Spawn{ type, count, boss });
}

std::pair<EnemySpawn, bool> UniformSpawner::next_enemy() {
    static std::unordered_map<EnemyType, float> enemy_delay{
        { EnemyType::Solder, 1.f},
        { EnemyType::Bike, 0.5f },
        { EnemyType::Pickup, 1.f },
        { EnemyType::Truck, 1.5f },
        { EnemyType::BTR, 2.f },
        { EnemyType::Tank, 2.f },
        { EnemyType::CruiserI, 5.f },
        { EnemyType::SmokeTruck, 1.5f },
        { EnemyType::MREW, 1.5f }
    };
    if (m_spawners.empty()) {
        return std::make_pair(EnemySpawn(), false);
    }
    auto it = m_spawners.begin();
    EnemySpawn spawn;
    spawn.type = it->type;
    spawn.boss = it->boss;
    spawn.delay = enemy_delay[it->type];
    --it->count;
    if (it->count == 0)
        m_spawners.erase(it);
    return { spawn, true };
}

std::string UniformSpawner::description() {
    std::map<EnemyType, int> enemies;
    for (auto& spw : m_spawners) {
        enemies[spw.type] += spw.count;
    }
    std::string description;
    for (auto& it = enemies.begin(); it != enemies.end(); ++it) {
        description += to_string(it->first) + " x" + std::to_string(it->second);
        if (it != --enemies.end())
            description += "\n";
    }
    return description;
}

void ControlledSpawner::add_enemy(EnemyType type, float delay, bool boss) {
    m_spawners.push_back(EnemySpawn{ type, delay, boss });
}

std::pair<EnemySpawn, bool> ControlledSpawner::next_enemy() {
    if (m_current_spawner >= m_spawners.size())
        return std::make_pair(EnemySpawn(), false);
    EnemySpawn sp = m_spawners[m_current_spawner];
    ++m_current_spawner;
    return std::make_pair(sp, true);
}

std::string ControlledSpawner::description() {
    std::map<EnemyType, int> enemies;
    for (auto& spw : m_spawners) {
        enemies[spw.type] += 1;
    }
    std::string description;
    for (auto& it = enemies.begin(); it != enemies.end(); ++it) {
        description += to_string(it->first) + " x" + std::to_string(it->second);
        if (it != --enemies.end())
            description += "\n";
    }
    return description;
}

std::list<size_t> get_random_enters(size_t enters_N, size_t requested_N) {
    std::list<size_t> list;
    for (size_t i = 0; i < enters_N; ++i)
        list.push_back(i);

    // Копируем в вектор
    std::vector<size_t> temp(list.begin(), list.end());
    // Перемешиваем
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(temp.begin(), temp.end(), gen);
    list.assign(temp.begin(), temp.end());

    while (list.size() > requested_N) {
        auto it = list.begin();
        std::advance(it, rand() % list.size());
        list.erase(it);
    }
    return list;
}

std::pair<size_t, size_t> two_random_enters() {
    auto list = get_random_enters(EnemyManager::Instance().all_paths.size(), 2);
    return { list.front(), list.back() };
}

size_t one_random_enter() {
    auto list = get_random_enters(EnemyManager::Instance().all_paths.size(), 2);
    return list.front();
}


RoadGraph::PathID get_random_route(int start_node_count) {
    auto all_paths = EnemyManager::Instance().all_paths;
    auto it = all_paths.begin();
    std::advance(it, start_node_count);
    RoadGraph::Node* start_node = it->first;
    int path = rand() % it->second.size();
    return RoadGraph::PathID{ start_node, path };
};

RoadGraph::PathID one_random_route() {
    return get_random_route(one_random_enter());
}

std::pair<RoadGraph::PathID, RoadGraph::PathID> random_routes_from_to_random_enters() {
    auto [e1, e2] = two_random_enters();
    return { get_random_route(e1), get_random_route(e2) };
}

WaveController::WaveController() {
    int all_paths = EnemyManager::Instance().all_paths.size();

    // Волна 0
    {
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = one_random_route();
        r1->add_spawner(EnemyType::SmokeTruck, 1);
        r1->add_spawner(EnemyType::MREW, 1);
        r1->add_spawner(EnemyType::Solder, 10);
        r1->add_spawner(EnemyType::SmokeTruck, 1);
        r1->add_spawner(EnemyType::Solder, 10);
        r1->add_spawner(EnemyType::Truck, 1);
        r1->add_spawner(EnemyType::Pickup, 1);
        r1->add_spawner(EnemyType::BTR, 1);
        r1->add_spawner(EnemyType::Tank, 1);
        r1->add_spawner(EnemyType::CruiserI, 1);
        r1->add_spawner(EnemyType::Bike, 1);
        Wave w;
        w.prepairing_time = 0;
        w.routes.push_back(std::move(r1));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }

    // Волна 1
    {
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = one_random_route();
        r1->add_spawner(EnemyType::Solder, 3);
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(std::move(r1));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }
    // Волна 2
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Solder, 3);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 3);
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }
    // Волна 3
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Solder, 6);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 6);
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 2000;
        m_waves.push_back(std::move(w));
    }
    // Волна 4
    {
        auto r1 = std::make_unique<ControlledSpawner>();
        r1->id = one_random_route();
        r1->add_enemy(EnemyType::Solder, 1);
        r1->add_enemy(EnemyType::Solder, 1);
        r1->add_enemy(EnemyType::Solder, 1.5);
        r1->add_enemy(EnemyType::Bike, 0);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }
    // Волна 5
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Solder, 5);
        r1->add_spawner(EnemyType::Bike, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 5);
        r2->add_spawner(EnemyType::Bike, 1);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }
    // Волна 6
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Pickup, 1);
        r1->add_spawner(EnemyType::Solder, 7);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 4);
        r2->add_spawner(EnemyType::Bike, 1);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }
    // Волна 7
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Pickup, 2);
        r1->add_spawner(EnemyType::Bike, 2);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 4);
        r2->add_spawner(EnemyType::Bike, 1);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1000;
        m_waves.push_back(std::move(w));
    }
    // Волна 8
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Truck, 1);
        r1->add_spawner(EnemyType::Pickup, 1);
        r1->add_spawner(EnemyType::Bike, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 5);
        r2->add_spawner(EnemyType::Pickup, 3);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1500;
        m_waves.push_back(std::move(w));
    }
    // Волна 9
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Truck, 5);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 5);
        r2->add_spawner(EnemyType::Pickup, 3);
        r2->add_spawner(EnemyType::Bike, 2);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1500;
        m_waves.push_back(std::move(w));
    }

    // Волна 10
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Solder, 10);
        r1->add_spawner(EnemyType::Bike, 4);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 10);
        r2->add_spawner(EnemyType::Bike, 4);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1500;
        m_waves.push_back(std::move(w));
    }
    // Волна 11
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::BTR, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::BTR, 1);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1500;
        m_waves.push_back(std::move(w));
    }
    // Волна 12
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Pickup, 3);
        r1->add_spawner(EnemyType::Truck, 3);
        r1->add_spawner(EnemyType::BTR, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Solder, 10);
        r2->add_spawner(EnemyType::BTR, 2);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 1500;
        m_waves.push_back(std::move(w));
    }
    // Волна 13
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Truck, 2);
        r1->add_spawner(EnemyType::BTR, 2);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Bike, 4);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 3000;
        m_waves.push_back(std::move(w));
    }
    // Волна 13
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::BTR, 3);
        r1->add_spawner(EnemyType::Tank, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Truck, 4);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 3000;
        m_waves.push_back(std::move(w));
    }
    // Волна 14
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Truck, 2);
        r1->add_spawner(EnemyType::BTR, 2);
        r1->add_spawner(EnemyType::Tank, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Tank, 1);
        r2->add_spawner(EnemyType::BTR, 3);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 3000;
        m_waves.push_back(std::move(w));
    }
    // Волна 15
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Tank, 3);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Tank, 3);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 3000;
        m_waves.push_back(std::move(w));
    }
    // Волна 16
    {
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = one_random_route();
        r1->add_spawner(EnemyType::Solder, 15);
        r1->add_spawner(EnemyType::Bike, 8);
        r1->add_spawner(EnemyType::Pickup, 8);
        r1->add_spawner(EnemyType::Truck, 8);
        r1->add_spawner(EnemyType::Tank, 8);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.reward = 5000;
        m_waves.push_back(std::move(w));
    }
    // Волна 17
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Bike, 10);
        r1->add_spawner(EnemyType::Tank, 14);
        r1->add_spawner(EnemyType::Truck, 18);
        r1->add_spawner(EnemyType::Solder, 20);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Bike, 10);
        r2->add_spawner(EnemyType::BTR, 20);
        r2->add_spawner(EnemyType::Truck, 10);
        r2->add_spawner(EnemyType::Pickup, 10);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 10000;
        m_waves.push_back(std::move(w));
    }
    // Волна 18
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Tank, 5);
        r1->add_spawner(EnemyType::Solder, 14);
        r1->add_spawner(EnemyType::Tank, 1);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::Tank, 5);
        r2->add_spawner(EnemyType::BTR, 3);
        r2->add_spawner(EnemyType::Truck, 5);
        r2->add_spawner(EnemyType::Pickup, 5);
        r2->add_spawner(EnemyType::CruiserI, 1, true);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 10000;
        m_waves.push_back(std::move(w));
    }

    // Волна 19
    {
        auto [id1, id2] = random_routes_from_to_random_enters();
        auto r1 = std::make_unique<UniformSpawner>();
        r1->id = id1;
        r1->add_spawner(EnemyType::Solder, 3);
        r1->add_spawner(EnemyType::Bike, 3);
        r1->add_spawner(EnemyType::Solder, 3);
        r1->add_spawner(EnemyType::Pickup, 5);
        r1->add_spawner(EnemyType::Solder, 5);
        r1->add_spawner(EnemyType::Bike, 5);
        r1->add_spawner(EnemyType::Pickup, 5);
        r1->add_spawner(EnemyType::Truck, 3);
        r1->add_spawner(EnemyType::Pickup, 3);
        r1->add_spawner(EnemyType::Truck, 3);
        r1->add_spawner(EnemyType::Tank, 8);
        r1->add_spawner(EnemyType::CruiserI, 2);
        auto r2 = std::make_unique<UniformSpawner>();
        r2->id = id2;
        r2->add_spawner(EnemyType::BTR, 20);
        r2->add_spawner(EnemyType::CruiserI, 2);
        Wave w;
        w.routes.push_back(std::move(r1));
        w.routes.push_back(std::move(r2));
        w.reward = 10000;
        m_waves.push_back(std::move(w));
    }
    
 
    set_active_wave(0);
}


void WaveController::logic(double dtime_microseconds) {
    auto& wave = m_waves[m_current_wave];
    switch (m_state) {
    case WaveController::State::Prepairing: {
        m_timer += dtime_microseconds;
        GameState::Instance().set_wave_info(std::to_string(m_current_wave + 1) + "/" + std::to_string(m_waves.size()) + " " + std::to_string(static_cast<int>(wave.prepairing_time - m_timer / (1000 * 1000.f))) + "s.");
        if (m_timer >= wave.prepairing_time * 1000 * 1000) {
            m_state = State::Spawn;
            m_timer = 0;
        }
        break;
    }
    case WaveController::State::Spawn: {
        bool all_of = true;
        for (auto& r : m_routes_states) {
            bool flag = r.logic(dtime_microseconds);
            all_of = all_of && flag;
        }
        if (all_of)
            m_state = WaveController::State::Completed;
        break;
    }
    default:
        break;
    }
}

void WaveController::start_wave() {
    if (m_state == WaveController::State::Prepairing) {
        m_state = State::Spawn;
        m_timer = 0;
        GameState::Instance().set_wave_info(std::to_string(m_current_wave + 1) + "/" + std::to_string(m_waves.size()) + " 0s.");
    }
}

WaveController::RouteState::RouteState(IRoute& spawner): route{spawner} {
    bool flag;
    std::tie(current_enemy_spawn, flag) = route.next_enemy();
}

bool WaveController::RouteState::logic(double dtime_ms) {
    if (completed)
        return true;
    timer += dtime_ms;
    if (timer >= current_enemy_spawn.delay * 1000 * 1000) {
        EnemyManager::Instance().spawn(current_enemy_spawn.type, route.id, current_enemy_spawn.boss);
        bool flag;
        std::tie(current_enemy_spawn, flag) = route.next_enemy();
        if (!flag) {
            completed = true;
            return true;
        }
        timer = 0;
    }
    return false;
}

bool WaveController::next_wave() {
    if (m_state != State::Completed)
        return true;
    GameState::Instance().player_coins_add(m_waves[m_current_wave].reward);
    return set_active_wave(++m_current_wave);
}

bool WaveController::set_active_wave(int wave) {
    if (wave >= m_waves.size())
        return false;
    m_current_wave = wave;
    m_state = State::Prepairing;
    m_timer = 0;
    m_routes_states.clear();
    auto& w = m_waves[m_current_wave];
    auto& game_state = GameState::Instance();
    auto& all_paths = EnemyManager::Instance().all_paths;
    game_state.delete_all_enters();
    for (auto& route : w.routes) {
        game_state.add_enter(route->id, route->description());
    }
    for (auto& r : w.routes)
        m_routes_states.push_back(RouteState(*r.get()));



    return true;
}
