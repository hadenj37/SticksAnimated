#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Keyframe.h"
#include "Hand.h"
#include "Bot.h"

using namespace std;
using glm::vec4;
using glm::mat4x3;

//--  global variable declarations --//
GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from

int keyPresses[256] = {0}; // only for English keyboards!


shared_ptr<Program> prog;
shared_ptr<Camera> camera;
float t0;

bool playerTurn;
//bool takingInput;
shared_ptr<Hand> playerRight;
shared_ptr<Hand> playerLeft;
shared_ptr<Hand> botRight;
shared_ptr<Hand> botLeft;

//vec4 coeffs; //coefficents for time control cubic
//float tNorm;
//float tMax = 3;
//float sMax;
//float sNorm;
//std::vector<std::pair<float, float> > usTable;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	if (playerTurn) {
		// j: select right hand
		if (key == (unsigned int)'j' && playerLeft->getValue() > 0) {
			playerRight->setState(Hand::selected);
			playerLeft->setState(Hand::deselected);
		}
		// f: select left hand
		if (key == (unsigned int)'f' && playerLeft->getValue() > 0) {
			playerLeft->setState(Hand::selected);
			playerRight->setState(Hand::deselected);
		}
		// t: attack across w/ left
		if (key == (unsigned int)'t' && playerLeft->getState() == Hand::selected) {
			playerLeft->setState(Hand::attackingAcross);
			playerLeft->attack(*botLeft);
		}
		// e(or r?): attack forward w/ left
		if (key == (unsigned int)'e' && playerLeft->getState() == Hand::selected) {
			playerLeft->setState(Hand::attackingForward);
			playerLeft->attack(*botRight);
		}
		// y: attack across w/ right
		if (key == (unsigned int)'y' && playerRight->getState() == Hand::selected) {
			playerRight->setState(Hand::attackingAcross);
			playerRight->attack(*botRight);
		}
		// i(or u?): attack forward w/ right
		if (key == (unsigned int)'i' && playerRight->getState() == Hand::selected) {
			playerRight->setState(Hand::attackingForward);
			playerRight->attack(*botLeft);
		}
		// space: bump
		if (key == (unsigned int)' ') {
			playerRight->setState(Hand::bumping);
			playerLeft->setState(Hand::bumping);
		}
	}
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl  = mods & GLFW_MOD_CONTROL;
		bool alt   = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

/*float sTOu(float s) {
	//find table index
	size_t index = 0;
	while(index < usTable.size()){
		if (s >= usTable.at(index).second) {
			index++;
		} else {
			break;
		}
	}
	float alpha = (s - usTable.at(index-1).second)/(usTable.at(index).second - usTable.at(index-1).second);
	return (1-alpha)*usTable.at(index-1).first + alpha*usTable.at(index).first;
}*/

static void init(){
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);

	keyPresses[(unsigned)'c'] = 1;
	
	// Program
	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "Cel_vert.glsl", RESOURCE_DIR + "Cel_frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addUniform("MVit");
	prog->addUniform("lightPos");
	prog->addUniform("ka");
	prog->addUniform("kd");
	prog->addUniform("ks");
	prog->addUniform("s");
	prog->setVerbose(false);
	
	// Camera
	camera = make_shared<Camera>();

	// Game
	playerTurn = true;
	//takingInput = false;

	// Hands
	playerRight = make_shared<Hand>(false);
	playerLeft = make_shared<Hand>(true);
	botRight = make_shared<Hand>(false);
	botLeft = make_shared<Hand>(true);

	playerRight->updateFingerVals();
	playerLeft->updateFingerVals();
	botRight->updateFingerVals();
	botLeft->updateFingerVals();
	
	// Initialize time & parameters
	glfwSetTime(0.0);
	t0 = 0;
	
	/*
	//make lookup table
	vec3 prevVec = kframes.at(0).getPosition();
	usTable.push_back(make_pair(0.0f,0.0f));
	for (ucat = 0.2f; ucat < (float)kframes.size() + 0.1f; ucat += 0.2f) {
		//calculate next position
		u = ucat - (int)floor(ucat);
		uvec = vec4(1,u,u*u,u*u*u);
		G = mat4x3(kframes.at(((int)floor(ucat)-1)%kframes.size()).getPosition(),
				   kframes.at(((int)floor(ucat))%kframes.size()).getPosition(),
				   kframes.at(((int)floor(ucat)+1)%kframes.size()).getPosition(),
				   kframes.at(((int)floor(ucat)+2)%kframes.size()).getPosition());
		Pvec =  G * (B * uvec);

		//calculate & store s
		s = glm::distance(prevVec,Pvec);
		s += usTable.back().second; //add distance from previous u
		usTable.push_back(make_pair(ucat,s));
		prevVec = Pvec;
	}
	sMax = usTable.back().second;

	//solve for time coefficients
	mat4 A(vec4(0,1,0.75f,0.125f),
		vec4(0,1,1,0.25f),
		vec4(0,1,1,0.5f),
		vec4(1,1,0,1));
	vec4 b(1,1,0,0);
	coeffs = vec4(glm::inverse(A) * b);
	*/

	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

void render(){
	// Update time & parameter(s)
	// Update time.
	double t1 = glfwGetTime();
	float dt = (t1 - t0);
	//t += dt;
	t0 = t1;
	playerRight->updateU(dt);
	playerRight->updateFingerModels(dt);
	playerLeft->updateU(dt); 
	playerLeft->updateFingerModels(dt);
	botRight->updateU(dt);	 
	botRight->updateFingerModels(dt);
	botLeft->updateU(dt);	 
	botLeft->updateFingerModels(dt);

	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Use the window size for camera.
	glfwGetWindowSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*if(keyPresses[(unsigned)'c'] % 2) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}*/glDisable(GL_CULL_FACE);
	if(keyPresses[(unsigned)'z'] % 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);
	
	prog->bind();
	GLSL::checkError(GET_FILE_LINE);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniformMatrix4fv(prog->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(inverse(transpose(MV->topMatrix()))));
	glUniform3f(prog->getUniform("lightPos"), 0.0f, 7.0f, 0.0f);
	glUniform3f(prog->getUniform("ka"), 0.0f, 0.0f, 0.6f);
	glUniform3f(prog->getUniform("kd"), 0.0f, 0.0f, 1.0f);
	glUniform3f(prog->getUniform("ks"), 1.0f, 1.0f, 1.0f);
	glUniform1f(prog->getUniform("s"), 500.0f);

	// Draw Hands
	MV->pushMatrix();
		playerRight->drawHand(MV, prog);
		playerLeft->drawHand(MV, prog);
		MV->pushMatrix();
			MV->rotate(M_PI,vec3(0.0f,1.0f,0.0f));
			glUniform3f(prog->getUniform("ka"), 0.6f, 0.0f, 0.0f);
			glUniform3f(prog->getUniform("kd"), 1.0f, 0.0f, 0.0f);
			botRight->drawHand(MV, prog);
			botLeft->drawHand(MV, prog);
		MV->popMatrix();
	MV->popMatrix();
	
	prog->unbind();
	/* Draw the frame and the grid with OpenGL 1.x(no GLSL) */
	// Setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(P->topMatrix()));
	
	// Setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(MV->topMatrix()));
	
	// Draw frame
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
	glLineWidth(1);
	// Draw grid
	float gridSizeHalf = 20.0f;
	int gridNx = 40;
	int gridNz = 40;
	glLineWidth(1);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINES);
	for(int i = 0; i < gridNx+1; ++i) {
		float alpha = i / (float)gridNx;
		float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(x, 0, -gridSizeHalf);
		glVertex3f(x, 0,  gridSizeHalf);
	}
	for(int i = 0; i < gridNz+1; ++i) {
		float alpha = i / (float)gridNz;
		float z = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(-gridSizeHalf, 0, z);
		glVertex3f( gridSizeHalf, 0, z);
	}
	glEnd();
	
	/*if(showFrames){
		//draw splines
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_STRIP);
		for (ucat = 0.0f; ucat <= (float)kframes.size() + 0.1f; ucat += 0.1f) {
			G = mat4x3(kframes.at(((int)floor(ucat) - 1) % kframes.size()).getPosition(),
				kframes.at(((int)floor(ucat)) % kframes.size()).getPosition(),
				kframes.at(((int)floor(ucat) + 1) % kframes.size()).getPosition(),
				kframes.at(((int)floor(ucat) + 2) % kframes.size()).getPosition());
			u = ucat - (int)floor(ucat);
			uvec = vec4(1.0f, u, u * u, u * u * u);
			Pvec = G * (B * uvec);
			glVertex3f(Pvec.x, Pvec.y, Pvec.z);
		}
		glEnd();

		
		//draw arc points
		glPointSize(10.0f);
		glBegin(GL_POINTS);
		for (s = 0.0f; s < sMax; s += 0.4f) {
			ucat = sTOu(s);
			G = mat4x3(kframes.at(((int)floor(ucat) - 1) % kframes.size()).getPosition(),
				kframes.at(((int)floor(ucat)) % kframes.size()).getPosition(),
				kframes.at(((int)floor(ucat) + 1) % kframes.size()).getPosition(),
				kframes.at(((int)floor(ucat) + 2) % kframes.size()).getPosition());
			u = ucat - (int)floor(ucat);
			uvec = vec4(1.0f, u, u * u, u * u * u);
			Pvec = G * (B * uvec);
			glVertex3f(Pvec.x, Pvec.y, Pvec.z);
		}
		glEnd();
		
	}*/

	// Pop modelview matrix
	glPopMatrix();
	
	// Pop projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(1920, 1080, "Sticks", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			// Render scene.
			render();
			// Swap front and back buffers.
			glfwSwapBuffers(window);
		}
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
