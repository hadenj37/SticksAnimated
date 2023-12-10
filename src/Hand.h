#pragma once
#ifndef HAND_H
#define HAND_H

#include "Finger.h"
#include "MatrixStack.h"
#include "Keyframe.h"
#include "Shape.h"
#include "Program.h"
#include <memory>
#include <cmath>

class Hand {
public:
	enum States {deselected = 0, selected, attackingForward, attackingAcross, bumping};
	Hand();
	Hand(std::string resourceDir ,bool lefty = false);
	virtual ~Hand() {}
	void drawHand(shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> drawProg);
	void countUp();
	void countDown();
	void attack(Hand& target);
	void setU(float t) { 
		uCat = (float)fmod(t, curveframes.size()-1); //time % max u value = [0,curfames.size()-1)
		uCat = std::min<float>(uCat, (float)curveframes.size()-1);
		u = uCat - (int)floor(uCat);
	}
	void updateU(float t) {
		uCat += t;
		uCat = std::min<float>(uCat, (float)curveframes.size()-1);
		u = uCat - (int)floor(uCat);
	}
	void resetU() {
		uCat = 0;
		u = 0;
	}
	float getUcat() { return uCat; }
	int getValue() { return fingersUp; }
	int getState() { return currentState; }
	void setState(States newState);
	//void playAnim(States newState);
	void updateFingerVals();
	void updateFingerModels(float t);
	void reset();

private:
	bool left;
	int fingersUp;
	float u = 0.0f; // middle u
	float uCat = 0.0f; // Big u

	States currentState = deselected;
	std::vector<Finger> fingers;
	std::vector<Keyframe> keyframes;
	std::vector<Keyframe> curveframes;
	std::shared_ptr<Shape> handModel;
	std::shared_ptr<Shape> fingerModel;
};

#endif