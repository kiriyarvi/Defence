#pragma once
#include <functional>
#include <SFML/Graphics.hpp>
#include "utils/animation.h"

struct MedBlustFramer: public ISpriteFramer {
	MedBlustFramer();
	void on_frame(int frame) override;
};

struct DoubleBlustFramer : public ISpriteFramer {
	DoubleBlustFramer();
	void on_frame(int frame) override;
};

struct DenceBlustFramer : public ISpriteFramer {
	DenceBlustFramer();
	void on_frame(int frame) override;
};
