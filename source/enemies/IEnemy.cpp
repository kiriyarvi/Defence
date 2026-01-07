#include "enemies/IEnemy.h"
#include "game_state.h"
#include "enemy_manager.h"
#include "covering_database.h"

std::string to_string(EnemyType type) {
    std::unordered_map<EnemyType, std::string> m{
        {EnemyType::Solder, "Пехотинец"},
        {EnemyType::Bike, "Мотоциклист"},
        {EnemyType::BTR, "БТР"},
        {EnemyType::CruiserI, "Наземный крейсер-I"},
        {EnemyType::Pickup, "Пикап"},
        {EnemyType::Truck, "Грузовик"},
        {EnemyType::Tank, "Танк"},
        {EnemyType::SmokeTruck, "Грузовик с дымовой завесой"}
    };
    return m[type];
}

void HealthIndicator::draw(sf::RenderWindow& window, float x, float y, float max_healf, float current_healf) {
	sf::RectangleShape rectangle(sf::Vector2f(width, 1));
	rectangle.setPosition(x, y);
	rectangle.setFillColor(sf::Color::Transparent);
	rectangle.setOutlineColor(sf::Color::Black);
	rectangle.setOutlineThickness(0.5f);
	rectangle.setOrigin(width / 2, 0.5);
	window.draw(rectangle);
	rectangle.setFillColor(fill_color);
	rectangle.setOutlineColor(sf::Color::Transparent);
	rectangle.setSize(sf::Vector2f(width * current_healf / max_healf, 1.f));
	rectangle.setOrigin(0, 0.5);
	rectangle.move(-width / 2, 0);
	window.draw(rectangle);
}

IEnemy::IEnemy(const ParamsManager::Params::Enemies::Enemy& p, EnemyType t, Collision c):
    params{ p }, type{ t }, collision{ c }
{
	health = params.health;
};

bool IEnemy::logic(double dtime) {
    //1. Определим засекреченность врага
    bool in_smoke = false;
    for (auto& s : EnemyManager::Instance().get_smokes()) {
        if (!s.active()) continue;
        float d = glm::distance(s.m_pos, position);
        if (d <= s.m_max_radius) {
            in_smoke = true;
            if (!m_in_smoke) { // если не был в дыму
                m_in_smoke = true; //статус изменилися
                CoveringDataBase::Instance().make_enemy_covered(id);
            }
            break; // если был в дыму, статус тоже не поменялся => в любом случае break.
        }
    }
    if (!in_smoke && m_in_smoke) { // вышли из завесы
        m_in_smoke = false;
        CoveringDataBase::Instance().remove_enemy(id);
    }
    //2. Логика починки
	if (repairing) {
		repairing_timer += dtime;
		if (repairing_timer >= repairing_time * 1000 * 1000)
			repairing = false;
		return false;
	}
    // 3. Логика перемещения по маршруту.
	sf::Vector2f current_pos(position.x, position.y);
	auto sf_dir = goal - current_pos;
	glm::vec2 dir = { sf_dir.x, sf_dir.y };
	float dist = glm::length(dir);
	dir = dir / dist;
	float potential = dtime * params.speed * 32;
	if (dist * 1000 * 1000 > potential) {
		float angle = glm::degrees(glm::atan(dir.y, dir.x));
		rotation = angle;
		dir = dir * potential / (1000 * 1000.f);
		position += glm::vec2(dir.x, dir.y);
	}
	else { 
		auto& enemy_manager = EnemyManager::Instance();
        auto& path = enemy_manager.all_paths[path_id.start_node][path_id.path];
		if (goal_path_node + 1 == path.size()) {
			path_is_completed = true;
            if (m_boss)
                GameState::Instance().kill_player();
            else
			    GameState::Instance().player_health_add(-1); //TODO отнимать cost
			return true;
		}
		position = glm::vec2(path[goal_path_node]->x * 32 + 16, path[goal_path_node]->y * 32 + 16);
		++goal_path_node;
		goal = sf::Vector2f(path[goal_path_node]->x * 32 + 16, path[goal_path_node]->y * 32 + 16);
	}
	return false;
}

bool IEnemy::break_enemy(double repairing_time) {
	sf::Vector2i current_cell(position.x / 32, position.y / 32);
	if (current_cell == last_breaking_cell)
		return false;
	repairing = true;
	this->repairing_time = repairing_time;
	repairing_timer = 0;
	last_breaking_cell = current_cell;
	return true;
}

void IEnemy::draw_effects(sf::RenderWindow& window) {
	if (!repairing)
		return;
	sf::Sprite repairing_wrench(TextureManager::Instance().textures[TextureID::RepairWrench]);
	repairing_wrench.setOrigin(16, 16);
	repairing_wrench.setPosition(position.x, position.y);
	repairing_wrench.setScale(0.3, 0.3);
	window.draw(repairing_wrench);
}

void IEnemy::post_smoke_effects(sf::RenderWindow& window) {
    if (m_in_smoke && CoveringDataBase::Instance().is_available_taget(id)) {
        draw_collision(window);
        draw_effects(window); // будем вызывать дважды, но ничего страшного.
    }
}

void IEnemy::draw_collision(sf::RenderWindow& window) {
    sf::Transform t;
    t.translate(position.x, position.y);
    t.rotate(rotation);

    sf::Sprite sp(TextureManager::Instance().textures[TextureID::Capture]);
    sp.setScale(m_bounding_box_border_scale, m_bounding_box_border_scale);
    sp.setTextureRect(sf::IntRect(0, 0, 8, 8));
    sp.setPosition(collision.tl_vertex.x, collision.tl_vertex.y);
    window.draw(sp, t);
    sp.setTextureRect(sf::IntRect(8, 0, 8, 8));
    sp.setPosition(collision.br_vertex.x, collision.tl_vertex.y);
    sp.setOrigin(8, 0);
    window.draw(sp, t);
    sp.setTextureRect(sf::IntRect(0, 8, 8, 8));
    sp.setPosition(collision.tl_vertex.x, collision.br_vertex.y);
    sp.setOrigin(0, 8);
    window.draw(sp, t);
    sp.setTextureRect(sf::IntRect(8, 8, 8, 8));
    sp.setPosition(collision.br_vertex.x, collision.br_vertex.y);
    sp.setOrigin(8, 8);
    window.draw(sp, t);
}
