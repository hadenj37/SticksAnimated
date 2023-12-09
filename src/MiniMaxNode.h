#pragma once
#ifndef MINIMAXNODE_H
#define MINIMAXNODE_H
#include <vector>
#include <string>
#include <memory>

struct State {
public:
	int PR;
	int PL;
	int BR;
	int BL;
	State() {
		PR = 1;
		PL = 1;
		BR = 1;
		BL = 1;
	}
	State(int pr, int pl, int br, int bl) {
		PR = pr;
		PL = pl;
		BR = br;
		BL = bl;
	}
};

class MiniMaxNode {
protected:
	bool max;
	State state;
	int depth;
	int utility;
	std::string label;
	std::vector< std::shared_ptr<MiniMaxNode> > children;
public:
	MiniMaxNode();
	MiniMaxNode(bool isMax, State s, int level, std::string desc);
	virtual ~MiniMaxNode();
	void generateChildren();
	std::vector< std::shared_ptr<MiniMaxNode> > getChildren() { return children; }
	int getUtility() {return this->utility;}
	std::string getLabel() {return this->label;}
	void calcUtility();
	bool isLeaf() { return children.empty(); }
	void genTree(int maxDepth);
};

#endif