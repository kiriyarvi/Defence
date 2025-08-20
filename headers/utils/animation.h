#pragma once
#include <functional>
#include <list>
#include <memory>

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
	bool started() { return m_started; }

	std::function<void()> on_start; // выполниться при старте анимации
	std::function<void()> on_end;   // выполнится в конце анимации
	std::function<void(double)> on_progress;

	void add_framer(int frames, const std::function<void(uint32_t)>& framer);
	Animation& add_subanimation(double start, double end, Animation&& animation);
private:
	void finish();
	void framers_logic(double progress);
	void subanimation_logic(double progress);
	bool m_started = false;
	double m_duration = 0;
	double m_elapsed_time = 0;

	struct Framer {
		int last_frame = -1;
		int frames;
		std::function<void(uint32_t)> callback;
	};

	struct SubAnimation {
		SubAnimation() = default;
		SubAnimation(SubAnimation&&) = default;
		SubAnimation& operator=(SubAnimation&&) = default;
		double start;
		double end;
		std::unique_ptr<Animation> animation;
	};

	std::list<Framer> m_framers;
	std::list<SubAnimation> m_subanimations;
};