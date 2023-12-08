#pragma once
#ifndef FINGER_H
#define FINGER_H

#include "Shape.h"
#include "MatrixStack.h"
#include <vector>
#include <memory>

struct Segment {
	float bendAngle = 0.0f;
	float maxBend = 90.0f;
};

class Finger {
private:	
	bool up;
	std::vector<Segment> segments = {};
	float u;

public:
	Finger();
	Finger(bool raised);
	virtual ~Finger() {}
	void setOpen(bool isOpen);
	void setU(float t);
	void updateU(float t);
	void resetU() {u = 0.0f;}
	void updateModel();
	Segment getSegment(size_t number);
};

#endif