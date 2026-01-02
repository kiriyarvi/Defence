#pragma once
#include <functional>
#include <list>
#include <memory>

#include <SFML/Graphics.hpp>

struct ISpriteFramer {
	using Ptr = std::unique_ptr<ISpriteFramer>;
	virtual void on_frame(int frame) = 0;
	int frames = 0;
	sf::Sprite sprite;
};

class Animation {
public:
	Animation(double duration) : m_duration(duration) {}
	Animation() = default;
	Animation(Animation&&) = default;
	Animation& operator=(Animation&&) = default;
	void logic(double dtime_milliseconds);
	double get_duration() { return m_duration; }
	double get_animation_progress();
	void start();
	bool started() { return m_state == State::Started; }
	void set_duration(double duration_microseconds) { m_duration = duration_microseconds; }
	void set_loop(bool loop) { m_loop = loop; }
	std::pair<Animation*, Animation*> split(float quotient);
	std::vector<Animation*> split(const std::vector<float>& times);

	std::function<void()> on_start; // выполниться при старте анимации
	std::function<void()> on_end;   // выполнится в конце анимации
	std::function<void(double)> on_progress;


	void add_framer(int frames, const std::function<void(uint32_t)>& framer);
	void add_framer(const ISpriteFramer::Ptr& framer);
	Animation& add_subanimation(double start_time, double end_time, Animation&& animation);
private:
	void progress_logic(double progress);
	void finish();
	void framers_logic(double progress);
	void subanimation_logic(double progress);

	enum State {
		WaitToStart,
		Started,
		Finished
	} m_state = State::WaitToStart;
	double m_duration = 0; // время, используется чтобы конвертироать прошедшее время в progress (нужно только при вызове logic).
	double m_elapsed_time = 0; // прошедшее время (нужно только при вызове logic).
	bool m_loop = false; // true, тогда после того, как дойдет до Finished, запуститься снова.
	struct Framer {
		int last_frame = -1;
		int frames;
		std::function<void(uint32_t)> callback;
	};

	struct SubAnimation {
		SubAnimation() = default;
		SubAnimation(SubAnimation&&) = default;
		SubAnimation& operator=(SubAnimation&&) = default;
		double start_progress; // прогресс родительской анимации, с которой начинае проигрываться поданимация
		double end_progress;
		std::unique_ptr<Animation> animation;
	};

	std::list<Framer> m_framers;
	std::list<SubAnimation> m_subanimations;
};
