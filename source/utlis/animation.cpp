#include "utils/animation.h"
#include <numeric>

void Animation::start() {
	m_state = State::Started;
	m_elapsed_time = 0;
	for (auto& framer : m_framers)
		framer.last_frame = -1;
	if (on_start) on_start();
}

std::pair<Animation*, Animation*> Animation::split(float quotient) {
	std::pair<Animation*, Animation*> p;
	p.first = &add_subanimation(0.0, quotient * m_duration, Animation());
	p.second = &add_subanimation(quotient * m_duration, m_duration, Animation());
	return p;
}

std::vector<Animation*> Animation::split(const std::vector<float>& times) {
	m_duration = std::accumulate(times.begin(), times.end(), 0);
	std::vector<Animation*> a;
	double begin = 0.0;
	double end = 0.0;
	for (auto& d : times) {
		end += d;
		a.push_back(&add_subanimation(begin, end, Animation()));
		begin = end;
	}
	return a;
}

void Animation::add_framer(int frames, const std::function<void(uint32_t)>& framer) {
	m_framers.push_back(Framer{ -1, frames, framer });
}

void Animation::add_framer(const ISpriteFramer::Ptr& framer) {
	Framer fr;
	fr.last_frame = -1;
	fr.frames = framer->frames;
	ISpriteFramer* raw_framer = framer.get();
	fr.callback = [raw_framer](int frame) {
		raw_framer->on_frame(frame);
	}; 
	m_framers.push_back(fr);
}

Animation& Animation::add_subanimation(double start_time, double end_time, Animation&& animation) {
	if (m_duration == 0.0)
		throw std::logic_error("m_duration was not initialized!");
	auto& sub = m_subanimations.emplace_back();
	sub.start_progress = start_time / m_duration;
	sub.end_progress = end_time / m_duration;
	sub.animation = std::make_unique<Animation>(std::move(animation));
	sub.animation->set_duration(1.);
	return *sub.animation.get();
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
		if (subs.animation->m_state == State::WaitToStart && progress >= subs.start_progress && progress <= subs.end_progress)
			subs.animation->start();
		subs.animation->progress_logic((progress - subs.start_progress) / (subs.end_progress - subs.start_progress));
	}
}

void Animation::finish() {
	if (m_state == State::Started) {
		m_elapsed_time = 0.0;
		if (on_end) on_end();
		framers_logic(1.0);
		for (auto& subs : m_subanimations)
			subs.animation->finish();
		if (m_loop)
			start();
		else
			m_state = State::Finished;
	}
}



void Animation::logic(double dtime_milliseconds) {
	if (m_state != State::Started) return;
	m_elapsed_time += dtime_milliseconds;
	progress_logic(get_animation_progress());
	
}

void Animation::progress_logic(double progress) {
	if (m_state != State::Started) return;
	if (progress > 1.0) {
		finish();
		return;
	}
	if (on_progress) on_progress(progress);
	framers_logic(progress);
	subanimation_logic(progress);
}

double Animation::get_animation_progress() {
	return m_elapsed_time / (m_duration * 1000 * 1000);
}
