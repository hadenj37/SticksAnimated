#pragma once
#ifndef MINIMAXNODE_H
#define MINIMAXNODE_H

struct State {
	int P1R;
	int P1L;
	int P2R;
	int P2L;
};

class MiniMaxNode {
protected:
	bool max;
	const State state;
	int depth;
	int utility;
	std::vector< std::shared_ptr<MiniMaxNode> > children = {};
	std::shared_ptr<MiniMaxNode> parent;
public:
	MiniMaxNode();
	MiniMaxNode(State s);
	virtual ~MiniMaxNode() {}
	std::vector< std::shared_ptr<MiniMaxNode> > generateChildren();
};

#endif