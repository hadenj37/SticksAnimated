#include "Bot.h"
#include <iostream>

using namespace std;

string Bot::makeMove(State currentState) {
	// Init Minimax tree
	MiniMaxNode root = MiniMaxNode(true, currentState, 0, "");

	// Generate tree to max depth
	root.genTree(this->maxDepth);

	// Return max child's label
	vector< shared_ptr<MiniMaxNode> > options = root.getChildren();
	for (int i = 0; i < options.size(); i++) {
		if (root.getUtility() == options.at(i)->getUtility()) {
			return options.at(i)->getLabel();
		}
	}

	return ""; // shouldn't get here, but just in case, the bot will just never go.
}