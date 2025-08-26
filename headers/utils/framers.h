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

struct SteamFramer : public ISpriteFramer {
	SteamFramer();
	void on_frame(int frame) override;
};

struct MinigunShootFramer : public ISpriteFramer {
	MinigunShootFramer();
	void on_frame(int frame) override;
};

struct MinigunHitFramer : public ISpriteFramer {
	MinigunHitFramer();
	void on_frame(int frame) override;
};

struct MinigunReboundFramer : public ISpriteFramer {
	MinigunReboundFramer();
	void on_frame(int frame) override;
};

struct MineBlastFramer : public ISpriteFramer {
	MineBlastFramer();
	void on_frame(int frame) override;
};

struct PickupBlastFramer : public ISpriteFramer {
    PickupBlastFramer();
    void on_frame(int frame) override;
};

