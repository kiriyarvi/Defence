#include "utils/animation.h"

void Animation::start() {
	m_started = true;
	m_elapsed_time = 0;
	if (on_start) on_start();
}

void Animation::add_framer(int frames, const std::function<void(uint32_t)>& framer) {
	m_framers.push_back(Framer{ -1, frames, framer });
}

Animation& Animation::add_subanimation(double start, double end, Animation&& animation) {
	auto& sub = m_subanimations.emplace_back();
	sub.start = start;
	sub.end = end;
	sub.animation = std::make_unique<Animation>(std::move(animation));
	sub.animation->m_duration = sub.end - sub.start;
	return *m_subanimations.emplace_back(std::move(sub)).animation.get();
}

void Animation::framers_logic(double progress) {
	for (auto& framer : m_framers) {
		int current_frame = std::min(int(progress * framer.frames), framer.frames - 1);
		for (int i = framer.last_frame + 1; i <= current_frame; ++i)
			framer.callback(i);
		framer.last_frame = current_frame;
	}
}

void Animation::subanimation_logic(double progress) {
	for (auto& subs : m_subanimations) {
		if (!subs.animation->started() && progress >= subs.start)
			subs.animation->start();
		subs.animation->logic(progress - subs.start);
	}
}

void Animation::finish() {
	if (m_started) {
		m_started = m_loop;
		m_elapsed_time = 0.0;
		if (on_end) on_end();
		framers_logic(1.0);
		for (auto& subs : m_subanimations)
			subs.animation->finish();
	}
}



void Animation::logic(double dtime_milliseconds) {
	if (!m_started) return;
	m_elapsed_time += dtime_milliseconds;
	double progress = get_animation_progress();
	if (progress > 1.0) {
		finish();
		return;
	}
	if (on_progress) on_progress(get_animation_progress());
	framers_logic(progress);
	subanimation_logic(progress);
	
}


double Animation::get_animation_progress() {
	return m_elapsed_time / (m_duration * 1000 * 1000);
}
