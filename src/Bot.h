#pragma once
#ifndef BOT_H
#define BOT_H

#include "MiniMaxNode.h"
#include <string>

class Bot {
private:
	int maxDepth = 3;

public:
	Bot(int searchDepth = 3) {maxDepth = searchDepth;}
	virtual ~Bot() {}
	std::string makeMove(State currentState);
};

#endif