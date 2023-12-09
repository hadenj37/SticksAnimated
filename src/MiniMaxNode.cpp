#include "MiniMaxNode.h"
#include <iostream>
//#include <string>
//#include <vector>
//#include <memory>

using namespace std;

MiniMaxNode::MiniMaxNode() {
	max = true;
	state = {1,1,1,1};
	depth = 0;
	utility = 0;
	label = "start";
	children = {};
}

MiniMaxNode::MiniMaxNode(bool isMax, State s, int level, string desc) {
	max = isMax;
	state = s;
	depth = level;
	utility = 0;
	label = desc;
	children = {};
}

MiniMaxNode::~MiniMaxNode() {

}

void MiniMaxNode::generateChildren() {
	if (this->max) {
		// Terminal
		if (state.BL == 0 && state.BR == 0 || state.PL == 0 && state.PR == 0) {
			children = {};
			return;
		}

		// Right across
		if ((state.BR > 0) && (state.PR > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State((state.PR + state.BR) % 5, state.PL, state.BR, state.BL), (this->depth + 1), "right across"));
		}

		// Right forward
		if ((state.BR > 0) && (state.PL > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR, (state.PL + state.BR) % 5, state.BR, state.BL), (this->depth + 1), "right forward"));
		}

		// Left forward
		if ((state.BL > 0) && (state.PR > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State((state.PR + state.BL) % 5, state.PL, state.BR, state.BL), (this->depth + 1), "left forward"));
		}

		// Left across
		if ((state.BL > 0) && (state.PL > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR, (state.PL + state.BL) % 5, state.BR, state.BL), (this->depth + 1), "left across"));
		}

		// Bump
		if (state.BL==0 || state.BR==0) {
			if ((state.BL > 0) && (state.BL % 2 == 0)) {
				children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR,state.PL,state.BL/2,state.BL/2), (this->depth + 1), "bump"));
			}
			else if ((state.BR > 0) && (state.BR % 2 == 0)) {
				children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR,state.PL,state.BR/2,state.BR/2), (this->depth + 1), "bump"));
			}
		}

	} else /* min */ {
		// Terminal
		if (state.BL == 0 && state.BR == 0 || state.PL == 0 && state.PR == 0) {
			children = {};
			return;
		}

		// Right across
		if ((state.BR > 0) && (state.PR > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR, state.PL, (state.PR + state.BR) % 5, state.BL), (this->depth + 1), "right across"));
		}

		// Left forward
		if ((state.BR > 0) && (state.PL > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR, state.PL, (state.PL + state.BR) % 5, state.BL), (this->depth + 1), "left forward"));
		}

		// Right forward
		if ((state.BL > 0) && (state.PR > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR, state.PL, state.BR, (state.PR + state.BL) % 5), (this->depth + 1), "right forward"));
		}

		// Left across
		if ((state.BL > 0) && (state.PL > 0)) {
			children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR, state.PL, state.BR, (state.PL + state.BL) % 5), (this->depth + 1), "left across"));
		}

		// Bump
		if (state.PL==0 || state.PR==0) {
			if ((state.PL > 0) && (state.PL % 2 == 0)) {
				children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PL/2,state.PL/2,state.BR,state.BL), (this->depth + 1), "bump"));
			}
			else if ((state.PR > 0) && (state.PR % 2 == 0)) {
				children.push_back(make_shared<MiniMaxNode>(!(this->max), State(state.PR/2,state.PR/2,state.BR,state.BL), (this->depth + 1), "bump"));
			}
		}
	}
}

void MiniMaxNode::calcUtility() {
	if (children.empty()) {
		// Terminal
		if (state.BL == 0 && state.BR == 0 || state.PL == 0 && state.PR == 0) {
			this->utility = (state.BL != 0)? 100 : -100;
		} else {
			this->utility = state.BL + state.BR - state.PR - state.PL;
		}
	}
	else if (this->max) {
		int maxUtil = children.at(0)->getUtility();
		for (int i = 1; i < children.size(); i++) {
			if(maxUtil < children.at(i)->getUtility())
				maxUtil = children.at(i)->getUtility();
		}
		this->utility = maxUtil;
	}
	else /* min */{
		int minUtil = children.at(0)->getUtility();
		for (int i = 1; i < children.size(); i++) {
			if(minUtil > children.at(i)->getUtility())
				minUtil = children.at(i)->getUtility();
		}
		this->utility = minUtil;
	}
}

void MiniMaxNode::genTree(int maxDepth) {
	if (this->depth < maxDepth) {
		// Recursive call on children
		if(this->children.size() == 0) // This check should never fail
			this->generateChildren();
		else
			cout << "Called genTree() on a node with children already generated" << endl;

		if (!(this->children.empty())) {
			for (int i = 0; i < this->children.size(); i++) {
				children.at(i)->genTree(maxDepth);
			}
		}
	}
	
	this->calcUtility();
}