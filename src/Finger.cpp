#include "Finger.h"
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

Finger::Finger() {
	this->up = false;
	u = 0.0f;
	segments = {};
	segments.push_back({0.0f, 90.0f});
	segments.push_back({0.0f, 67.5f});
	segments.push_back({0.0f, 67.5f});
}

Finger::Finger(bool raised) {
	this->up = raised;
	u = raised? 1.0f : 0.0f;
	segments = {};
	segments.push_back({0.0f, 90.0f});
	segments.push_back({0.0f, 67.5f});
	segments.push_back({0.0f, 67.5f});
}

void Finger::updateModel() {
	assert(0.0f <= u);
	assert(1.0f >= u);

	if (this->up) {	// Finger should be up, not bent
		for (int i = 0 ; i < segments.size(); i++) {
			segments.at(i).bendAngle = u*0.0f + (1-u)*segments.at(i).maxBend;
		}
	}
	else {	// Finger should be down
		for (int i = 0 ; i < segments.size(); i++) {
			segments.at(i).bendAngle = u*segments.at(i).maxBend + (1-u)*0.0f;
		}
	}
}

void Finger::setU(float t) {
	assert(0.0f <= t);
	this->u = t;
	u = std::min<float>(u, 1.0f);
}

void Finger::updateU(float t) {
	assert(0.0f <= t);
	this->u += t;
	u = std::min<float>(u, 1.0f);
}

void Finger::setOpen(bool isOpen) { 
	if (isOpen != this->up) {
		this->up = isOpen;
		this->resetU();
	}
}

Segment Finger::getSegment(size_t number) {
	return segments.at(number);
}