#pragma once
#ifndef KEYFRAME_H
#define KEYFRAME_H

#define M_PI 3.14159265358979323846   // pi

#include <vector>
#include <string>
#include <memory>
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using std::vector;
using std::string;
using std::shared_ptr;
using std::make_shared;
using glm::vec3;
using glm::quat;
using glm::mat4;


class Keyframe {
private:
	vec3 pos;
	quat rot;

public:
	//getters
	vec3 getPosition() {return pos;}
	quat getQuaternion() {return rot;}

	//structors
	Keyframe() {
		pos = vec3(0.0f, 0.0f, 0.0f);
		rot = glm::angleAxis(0.0f, vec3(0.0f,1.0f,0.0f));
	}

	Keyframe(vec3 position, quat orientation) {
		pos = position;
		rot = orientation;
	}

	virtual ~Keyframe() {}

};


#endif