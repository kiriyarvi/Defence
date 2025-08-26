#include "guns/mine.h"
#include "utils/framers.h"
#include "enemy_manager.h"
#include "sound_manager.h"

Mine::Mine(): m_params(ParamsManager::Instance().params.guns.mine) {
	m_mine_sprite.setTexture(TileMap::Instance().textures[TileTexture::Mine]);
	m_mine_sprite.setOrigin(8, 8);

	m_blast_framer = std::make_unique<MineBlast>();
	m_blast_animation.set_duration(1.0);
	m_blast_animation.add_framer(m_blast_framer);
}

void Mine::draw(sf::RenderWindow& window, int x_id, int y_id) {
	if (m_state == State::Ready) {
		m_mine_sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
		window.draw(m_mine_sprite);
	}
}

void Mine::draw_effects(sf::RenderWindow& window, int x_id, int y_id)  {
	if (m_blast_animation.started()) {
		m_blast_framer->sprite.setPosition(x_id * 32 + 16, y_id * 32 + 16);
		window.draw(m_blast_framer->sprite);
	}
}

struct EnemyD {
	IEnemy* enemy = nullptr;
	double distance;
};

void Mine::logic(double dtime_microseconds, int x_id, int y_id) {
	if (m_state == State::Ready) {
		glm::vec2 pos(x_id * 32 + 16, y_id * 32 + 16);
		auto& enemies = EnemyManager::Instance().m_enemies;
		std::list<EnemyD> in_damage_radius;
		for (auto& enemy : enemies) {
			double distance = glm::length(enemy->get_position() - pos) / 32;
			if (distance <= m_params.activation_radius) {
				m_state = State::Activated;
				SoundManager::Instance().play(Sounds::MineBlast);
				m_blast_animation.start();
				m_blast_animation.logic(0.0);
			}
			if (distance < m_params.damage_radius)
				in_damage_radius.push_back(EnemyD{ enemy.get(), distance });
		}
		if (m_state == State::Activated) {
			for (auto& enemy : in_damage_radius) {
                if (enemy.enemy->params.armor_level > m_params.armor_penetration_level)
                    return;
				float p = 1. - std::max(enemy.distance - m_params.activation_radius, 0.0) / (m_params.damage_radius - m_params.activation_radius);
				enemy.enemy->health -= m_params.min_damage + p * (m_params.max_damage - m_params.min_damage);
			}
		}
	}
	else if (m_state == State::Activated) {
		m_blast_animation.logic(dtime_microseconds);
		if (!m_blast_animation.started())
			m_state = State::Destroyed;
	}

}
