#include "wave_controller.h"
#include "enemy_manager.h"
#include "game_state.h"

WaveController::WaveController() {
    int all_paths = EnemyManager::Instance().all_paths.size();
    // Волна 1
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.reward = 500;
        m_waves.push_back(w);
    }
    // Волна 2
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 500;
        m_waves.push_back(w);
    }
    // Волна 3
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 2000;
        m_waves.push_back(w);
    }
    // Волна 4
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1.5 });
        r1.spawner.push_back({ EnemyType::Bike, 0.5 });
        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 5
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1.5 });
        r1.spawner.push_back({ EnemyType::Bike, 0.5 });
        Wave w;
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Bike, 0.5 });
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 6
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        Wave w;
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Bike, 0.5 });
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 7
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1.5 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Bike, 0.5 });

        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 8
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Truck, 1.5 });
        r1.spawner.push_back({ EnemyType::Pickup, 1.5 });
        r1.spawner.push_back({ EnemyType::Bike, 1.5 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });

        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 9
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1.5 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });

        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 10
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Solder, 1.5 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Bike, 1 });
        r2.spawner.push_back({ EnemyType::Bike, 1 });

        Wave w;
        w.prepairing_time = 3;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 11
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 12
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 13
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Bike, 1 });
        r2.spawner.push_back({ EnemyType::Bike, 1 });
        r2.spawner.push_back({ EnemyType::Bike, 1 });
        r2.spawner.push_back({ EnemyType::Bike, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 1000;
        m_waves.push_back(w);
    }
    // Волна 14
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 3000;
        m_waves.push_back(w);
    }
    // Волна 15
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        r1.spawner.push_back({ EnemyType::BTR, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 4000;
        m_waves.push_back(w);
    }
    // Волна 16
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        Route r2;
        r2.route = rand() % all_paths;
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::Tank, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 1 });
        r2.spawner.push_back({ EnemyType::BTR, 3 });
        r2.spawner.push_back({ EnemyType::Truck, 1 });
        r2.spawner.push_back({ EnemyType::Truck, 1 });
        r2.spawner.push_back({ EnemyType::Truck, 1 });
        r2.spawner.push_back({ EnemyType::Truck, 3 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        r2.spawner.push_back({ EnemyType::Pickup, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.routes.push_back(r2);
        w.reward = 4000;
        m_waves.push_back(w);
    }
    // Волна 17
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::CruiserI, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.reward = 10000;
        m_waves.push_back(w);
    }
    // Волна 18
    {
        Route r1;
        r1.route = rand() % all_paths;
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Solder, 0.5 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Solder, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Bike, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Pickup, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Truck, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::Tank, 1 });
        r1.spawner.push_back({ EnemyType::CruiserI, 1 });
        r1.spawner.push_back({ EnemyType::CruiserI, 1 });
        Wave w;
        w.prepairing_time = 5;
        w.routes.push_back(r1);
        w.reward = 10000;
        m_waves.push_back(w);
    }
    set_active_wave(0);
}


void WaveController::logic(double dtime_microseconds) {
    auto& wave = m_waves[m_current_wave];
    switch (m_state) {
    case WaveController::State::Prepairing: {
        m_timer += dtime_microseconds;
        if (m_timer >= wave.prepairing_time * 1000 * 1000) {
            m_state = State::Spawn;
            m_timer = 0;
        }
        break;
    }
    case WaveController::State::Spawn: {
        bool all_of = true;
        for (auto& r : m_routes_states) {
            all_of = all_of && r.logic(dtime_microseconds);
        }
        if (all_of)
            m_state = WaveController::State::Completed;
        break;
    }
    default:
        break;
    }
}

bool WaveController::RouteState::logic(double dtime_ms) {
    if (current_spawner >= route.spawner.size())
        return true;
    timer += dtime_ms;
    if (timer >= route.spawner[current_spawner].delay * 1000 * 1000) {
        EnemyManager::Instance().spawn(route.spawner[current_spawner].type, route.route);
        ++current_spawner;
        timer = 0;
        return current_spawner >= route.spawner.size();
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
    for (auto& r : w.routes)
        m_routes_states.push_back({ 0,0, r });
    return true;
}
