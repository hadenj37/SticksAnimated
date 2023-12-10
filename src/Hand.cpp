#include "Hand.h"
#include "GLSL.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace glm;
using namespace std;

Hand::Hand() {
	throw std::logic_error("Default not designed to be used at present");
}

Hand::Hand(string resourceDir, bool lefty) {
	left = lefty;
	fingersUp = 1;
	u = 0.0f;
	uCat = 0.0f;
	currentState = deselected;
	
	// Make fingers
	fingers = {};
	fingers.push_back(Finger(true)); // first
	fingers.push_back(Finger()); // second
	fingers.push_back(Finger()); // third
	fingers.push_back(Finger()); // fourth


	//-- Make keyframes --//
	float mirrorFactor = this->left? -1.0f : 1.0f; // If left handed, mirror x-axis:	-x <-  -z ^v z  -> x
	keyframes = {};
	// Rest position
	keyframes.push_back(Keyframe(vec3(0.0f, 0.0f, 0.0f), glm::angleAxis(0.0f, vec3(0.0f, 1.0f, 0.0f)))); // 0
	// Bump
	keyframes.push_back(Keyframe(vec3(-5.0f, 0.0f, 0.0f), glm::angleAxis(0.0f, vec3(0.0f, 1.0f, 0.0f)))); // 1
	//Attack Forward
	keyframes.push_back(Keyframe(vec3(0.0f, 2.0f, 0.0f), glm::angleAxis(0.0f, vec3(0.0f, 1.0f, 0.0f)))); // Rise: 2
	keyframes.push_back(Keyframe(vec3(0.0f, 3.0f, -9.0f), glm::angleAxis((float)M_PI/4, vec3(1.0f, 0.0f, 0.0f)))); // Above target: 3
	keyframes.push_back(Keyframe(vec3(0.0f, -1.0f, -9.0f), glm::angleAxis(-(float)M_PI/4, vec3(1.0f, 0.0f, 0.0f)))); // Follow through: 4
	//Attack Across
	keyframes.push_back(Keyframe(vec3(-8.5f, 3.0f, -10.0f), glm::angleAxis(mirrorFactor*(float)M_PI/4, vec3(0.0f, 1.0f, 0.0f))*keyframes.at(3).getQuaternion())); // Above target: 5
	keyframes.push_back(Keyframe(vec3(-8.5f, -1.0f, -10.0f), glm::angleAxis(mirrorFactor*(float)M_PI/4, vec3(0.0f, 1.0f, 0.0f))*keyframes.at(4).getQuaternion())); // Follow through: 6
	
	// Set default curve
	curveframes = {};
	curveframes.push_back(keyframes.at(2));
	curveframes.push_back(keyframes.at(0));

	// Load model(s)
	handModel = make_shared<Shape>();
	handModel->loadMesh(resourceDir + "handbase.obj");
	handModel->init();

	fingerModel = make_shared<Shape>();
	fingerModel->loadMesh(resourceDir + "fingersegment.obj");
	fingerModel->init();
}

void Hand::drawHand(shared_ptr<MatrixStack> MV, const shared_ptr<Program> drawProg) {
	// Necessary vars
	mat4 B = { vec4(0,1,0,0),
				vec4(-0.5,0,0.5,0),
				vec4(1,-2.5,2,-0.5),
				vec4(-0.5,1.5,-1.5,0.5) }; //Catmull-Romm spline matrix
	mat4 E; //stores quaternions
	mat4x3 G; //stores positions
	vec4 uvec; //vector of u
	vec3 Pvec; //interpolated position
	vec4 qVec; //interpolated rotation
	quat q; //temp quaternion
	mat4 interpRotation; //interpolated rotation matrix
	float mirrorFactor = this->left? -1.0f : 1.0f; // If left handed, mirror x-axis:	-x <-  -z ^v z  -> x
	
	uvec = vec4(1,u,u*u,u*u*u);
	int uFloor = floor(uCat);
	G = mat4x3(curveframes.at((uFloor < 2 ? 0 : (uFloor - 1))).getPosition(),
				curveframes.at( uFloor ).getPosition(),
				curveframes.at( (size_t)std::min(uFloor + 1, (int)(curveframes.size()-1)) ).getPosition(),
				curveframes.at( (size_t)std::min(uFloor + 2, (int)(curveframes.size()-1)) ).getPosition());
	Pvec =  G * (B * uvec);
	Pvec.x *= mirrorFactor;

	// Fill E with rotation quaternion of the 4 control frames
	E = mat4(vec4(curveframes.at((uFloor < 2 ? 0 : (uFloor - 1))).getQuaternion().x,
				  curveframes.at((uFloor < 2 ? 0 : (uFloor - 1))).getQuaternion().y,
				  curveframes.at((uFloor < 2 ? 0 : (uFloor - 1))).getQuaternion().z,
				  curveframes.at((uFloor < 2 ? 0 : (uFloor - 1))).getQuaternion().w),
			 vec4(curveframes.at(uFloor).getQuaternion().x,
				  curveframes.at(uFloor).getQuaternion().y,
				  curveframes.at(uFloor).getQuaternion().z,
				  curveframes.at(uFloor).getQuaternion().w),
			 vec4(curveframes.at((size_t)std::min(uFloor + 1, (int)(curveframes.size()-1))).getQuaternion().x,
				  curveframes.at((size_t)std::min(uFloor + 1, (int)(curveframes.size()-1))).getQuaternion().y,
				  curveframes.at((size_t)std::min(uFloor + 1, (int)(curveframes.size()-1))).getQuaternion().z,
				  curveframes.at((size_t)std::min(uFloor + 1, (int)(curveframes.size()-1))).getQuaternion().w),
			 vec4(curveframes.at(std::min(uFloor + 2, (int)(curveframes.size()-1))).getQuaternion().x,
				  curveframes.at(std::min(uFloor + 2, (int)(curveframes.size()-1))).getQuaternion().y,
				  curveframes.at(std::min(uFloor + 2, (int)(curveframes.size()-1))).getQuaternion().z,
				  curveframes.at(std::min(uFloor + 2, (int)(curveframes.size()-1))).getQuaternion().w));
	//check dot products
	for(int column = 0; column < 3; column++) {
		if (glm::dot(E[column], E[column + 1]) < 0) {
			E[column+1] = -E[column+1];
		}
	}
	qVec = E * (B * uvec);
	q = quat(qVec[3], qVec[0], qVec[1], qVec[2]); // Constructor argument order: (w, x, y, z)
	interpRotation = glm::mat4_cast(glm::normalize(q)); // Creates a rotation matrix

	//- Palm -//
	// Start from world origin
	MV->pushMatrix();
		MV->translate(mirrorFactor*10,0,12); // To hand origin
		MV->translate(Pvec);
		MV->multMatrix(interpRotation);
		MV->pushMatrix();
			//MV->scale(8.0f, 2.0f, 7.0f);
			glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
			handModel->draw(drawProg);
		MV->popMatrix();

		//- Fingers -//
		// One
		MV->pushMatrix();
			// Base segment
			MV->translate(-3.0f*mirrorFactor, 0.0f, -2.5f); // To knuckle (rotation point)
			MV->rotate(fingers.at(0).getSegment(0).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Middle segment
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(0).getSegment(1).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Fingertip
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(0).getSegment(2).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
		MV->popMatrix();

		// Two
		MV->pushMatrix();
			// Base segment
			MV->translate(-1.0f*mirrorFactor, 0.0f, -2.5f); // To knuckle (rotation point)
			MV->rotate(fingers.at(1).getSegment(0).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Middle segment
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(1).getSegment(1).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Fingertip
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(1).getSegment(2).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
		MV->popMatrix();

		// Three
		MV->pushMatrix();
			// Base segment
			MV->translate(1.0f*mirrorFactor, 0.0f, -2.5f); // To knuckle (rotation point)
			MV->rotate(fingers.at(2).getSegment(0).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Middle segment
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(2).getSegment(1).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Fingertip
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(2).getSegment(2).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
		MV->popMatrix();

		// Four
		MV->pushMatrix();
			// Base segment
			MV->translate(3.0f*mirrorFactor, 0.0f, -2.5f); // To knuckle (rotation point)
			MV->rotate(fingers.at(3).getSegment(0).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Middle segment
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(3).getSegment(1).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Fingertip
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(fingers.at(3).getSegment(2).bendAngle*M_PI/180.0, vec3(-1.0f,0.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
		MV->popMatrix();

		// Thumb
		MV->pushMatrix();
			// Base segment
			MV->translate(-3.0f*mirrorFactor, 0.0f, 2.5f); // To knuckle (rotation point)
			MV->rotate(mirrorFactor*(float)M_PI/8, vec3(0.0f, 1.0f, 0.0f)); // rotate
			MV->translate(0.0f,0.0f,-2.0f); // To center
			MV->pushMatrix();
				MV->scale(1.0f,1.0f,1.5f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Middle segment
			MV->translate(0.0f,0.0f,-2.0f); // To joint
			MV->rotate(-mirrorFactor*(float)M_PI/8, vec3(0.0f, 1.0f, 0.0f)); // rotate
			MV->rotate(-(float)M_PI/2, vec3(1.0f, 0.0f, 0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
			// Fingertip
			MV->translate(0.0f,0.0f,-1.0f); // To joint
			MV->rotate(-mirrorFactor*(float)M_PI/2, vec3(0.0f,1.0f,0.0f)); // rotate
			MV->translate(0.0f,0.0f,-1.0f); // To center
			MV->pushMatrix();
				//MV->scale(2.0f,2.0f,4.0f);
				glUniformMatrix4fv(drawProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(drawProg->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
				fingerModel->draw(drawProg);
			MV->popMatrix();
		MV->popMatrix();
	MV->popMatrix();
	GLSL::checkError(GET_FILE_LINE);
}

void Hand::countUp() {
	fingersUp = (fingersUp + 1) % 5;
	this->updateFingerVals();
}

void Hand::countDown() {
	fingersUp = (fingersUp - 1) % 5;
	this->updateFingerVals();
}

void Hand::attack(Hand& target) {
	for (int count = 0; count < this->fingersUp; count++) {
		target.countUp();
	}
}

void Hand::setState(States newState) {
	if (newState != currentState) {
		// Update current keyframe curve
		curveframes.clear();
		switch (newState) {
		case deselected:
			if (currentState == selected) this->resetU();
			curveframes.push_back(keyframes.at(2));
			curveframes.push_back(keyframes.at(0));
			break;
		case selected:
			this->resetU();
			curveframes.push_back(keyframes.at(0));
			curveframes.push_back(keyframes.at(2));
			break;
		case bumping:
			this->resetU();
			curveframes.push_back(keyframes.at(0));
			curveframes.push_back(keyframes.at(1));
			curveframes.push_back(keyframes.at(0));
			break;
		case attackingForward:
			this->resetU();
			curveframes.push_back(keyframes.at(2));
			curveframes.push_back(keyframes.at(3));
			curveframes.push_back(keyframes.at(4));
			curveframes.push_back(keyframes.at(0));
			break;
		case attackingAcross:
			this->resetU();
			curveframes.push_back(keyframes.at(2));
			curveframes.push_back(keyframes.at(5));
			curveframes.push_back(keyframes.at(6));
			curveframes.push_back(keyframes.at(0));
			break;
		default:
			break;
		}

		this->currentState = newState;
	}
}

void Hand::updateFingerVals() {
	for (int i = 0; i < 4; i++) {
		if (i + 1 <= this->fingersUp) {
			fingers.at(i).setOpen(true);
		}
		else {
			fingers.at(i).setOpen(false);
		}
	}
}

void Hand::updateFingerModels(float t) {
	for (int i = 0 ; i < fingers.size(); i++) {
		fingers.at(i).updateU(t);
		fingers.at(i).updateModel();
	}
}

void Hand::reset() {
	fingersUp = 1;
	u = 0.0f;
	uCat = 0.0f;
	setState(deselected);
	updateFingerVals();
	for (int i = 0 ; i < fingers.size(); i++) {
		fingers.at(i).setU((i==0? 1.0f : 0.0f));
	}
}