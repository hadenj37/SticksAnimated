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
bool gameOver;
shared_ptr<Hand> playerRight;
shared_ptr<Hand> playerLeft;
Bot bot;
shared_ptr<Hand> botRight;
shared_ptr<Hand> botLeft;


static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {
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
	if (action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl  = mods & GLFW_MOD_CONTROL;
		bool alt   = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

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
	gameOver = false;
	bot = Bot(3);
	
	// Hands
	playerRight = make_shared<Hand>(RESOURCE_DIR, false);
	playerLeft = make_shared<Hand>(RESOURCE_DIR, true);
	botRight = make_shared<Hand>(RESOURCE_DIR, false);
	botLeft = make_shared<Hand>(RESOURCE_DIR, true);

	playerRight->updateFingerVals();
	playerLeft->updateFingerVals();
	botRight->updateFingerVals();
	botLeft->updateFingerVals();
	
	// Initialize time
	glfwSetTime(0.0);
	t0 = 0;
	
	GLSL::checkError(GET_FILE_LINE);
}

void restart() {
	// Game
	playerTurn = true;
	//gameOver = false;
	bot = Bot(3);

	// Hands
	playerRight->reset();
	playerLeft->reset();
	botRight->reset();
	botLeft->reset();

	// Initialize time
	glfwSetTime(0.0);
	t0 = 0;
}

void render(){
	// Update time & parameters
	// Update time.
	double t1 = glfwGetTime();
	float dt = 1.5f * (t1 - t0);
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
	if (keyPresses[(unsigned)'c'] % 2) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if (keyPresses[(unsigned)'z'] % 2) {
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
	glUniform3f(prog->getUniform("lightPos"), 0.0f, 1.0f, 0.0f);
	glUniform3f(prog->getUniform("ka"), 0.0f, 0.0f, 0.4f);
	glUniform3f(prog->getUniform("kd"), 0.0f, 0.0f, 1.0f);
	glUniform3f(prog->getUniform("ks"), 1.0f, 1.0f, 1.0f);
	glUniform1f(prog->getUniform("s"), 100.0f);

	// Draw Hands
	MV->pushMatrix();
		playerRight->drawHand(MV, prog);
		playerLeft->drawHand(MV, prog);
		MV->pushMatrix();
			MV->rotate(M_PI,vec3(0.0f,1.0f,0.0f));
			glUniform3f(prog->getUniform("ka"), 0.4f, 0.0f, 0.0f);
			glUniform3f(prog->getUniform("kd"), 1.0f, 0.0f, 0.0f);
			botRight->drawHand(MV, prog);
			botLeft->drawHand(MV, prog);
		MV->popMatrix();
	MV->popMatrix();
	prog->unbind();
	
	// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	if (playerTurn && !gameOver) {
		// j: select right hand
		if (key == (unsigned int)'j' && playerRight->getValue() > 0) {
			playerRight->setState(Hand::selected);
			playerLeft->setState(Hand::deselected);
		}
		// f: select left hand
		if (key == (unsigned int)'f' && playerLeft->getValue() > 0) {
			playerLeft->setState(Hand::selected);
			playerRight->setState(Hand::deselected);
		}
		// t: attack across w/ left
		if (key == (unsigned int)'t' && playerLeft->getState() == Hand::selected && (botLeft->getValue() > 0)) {
			playerTurn = false;
			playerLeft->setState(Hand::attackingAcross);
			while (playerLeft->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerLeft->attack(*botLeft);

			// Reset to deselected after
			while (playerLeft->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerLeft->setState(Hand::deselected);
		}
		// r: attack forward w/ left
		if (key == (unsigned int)'r' && playerLeft->getState() == Hand::selected && (botRight->getValue() > 0)) {
			playerTurn = false;
			playerLeft->setState(Hand::attackingForward);
			while (playerLeft->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerLeft->attack(*botRight);

			// Reset to deselected after
			while (playerLeft->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerLeft->setState(Hand::deselected);
		}
		// y: attack across w/ right
		if (key == (unsigned int)'y' && playerRight->getState() == Hand::selected && (botRight->getValue() > 0)) {
			playerTurn = false;
			playerRight->setState(Hand::attackingAcross);
			while (playerRight->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerRight->attack(*botRight);

			// Reset to deselected after
			while (playerRight->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerRight->setState(Hand::deselected);
		}
		// u: attack forward w/ right
		if (key == (unsigned int)'u' && playerRight->getState() == Hand::selected && (botLeft->getValue() > 0)) {
			playerTurn = false;
			playerRight->setState(Hand::attackingForward);
			while (playerRight->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerRight->attack(*botLeft);

			// Reset to deselected after
			while (playerRight->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerRight->setState(Hand::deselected);
		}
		// spacebar: bump
		if ((key == (unsigned int)' ') && ( ((playerLeft->getValue() == 0) && (playerRight->getValue() % 2 == 0)) || ((playerRight->getValue() == 0) && (playerLeft->getValue() % 2 == 0)) )) {
			playerTurn = false;
			playerRight->setState(Hand::bumping);
			playerLeft->setState(Hand::bumping);
			if (playerLeft->getValue() > 0) {
				int newVal = playerLeft->getValue() / 2;
				for (int i = 0; i < newVal; i++) {
					playerLeft->countDown();
					playerRight->countUp();
				}
			}
			else /* playerRight > 0 */ {
				int newVal = playerRight->getValue() / 2;
				for (int i = 0; i < newVal; i++) {
					playerRight->countDown();
					playerLeft->countUp();
				}
			}

			// Reset to deselected after
			while (playerRight->getUcat() < 2.0f && !glfwWindowShouldClose(window)) {
				if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
					// Render scene.
					render();
					// Swap front and back buffers.
					glfwSwapBuffers(window);
				}
			}
			playerLeft->setState(Hand::deselected);
			playerRight->setState(Hand::deselected);
		}

		if (!playerTurn) {
			if ((botRight->getValue() > 0 || botLeft->getValue() > 0)){
				// Bot turn
				string botMove = bot.makeMove({ playerRight->getValue(),playerLeft->getValue(),botRight->getValue(),botLeft->getValue() });

				if (botMove == "right across") {
					//Select right
					botRight->setState(Hand::selected);
					while (botRight->getUcat() < 1.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}

					botRight->setState(Hand::attackingAcross);
					while (botRight->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botRight->attack(*playerRight);

					// Reset to deselected after
					while (botRight->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botRight->setState(Hand::deselected);
				}
				if (botMove == "right forward") {
					//Select right
					botRight->setState(Hand::selected);
					while (botRight->getUcat() < 1.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}

					botRight->setState(Hand::attackingForward);
					while (botRight->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botRight->attack(*playerLeft);

					// Reset to deselected after
					while (botRight->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botRight->setState(Hand::deselected);
				}
				if (botMove == "left across") {
					//Select left
					botLeft->setState(Hand::selected);
					while (botLeft->getUcat() < 1.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}

					botLeft->setState(Hand::attackingAcross);
					while (botLeft->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botLeft->attack(*playerLeft);

					// Reset to deselected after
					while (botLeft->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botLeft->setState(Hand::deselected);
				}
				if (botMove == "left forward") {
					//Select left
					botLeft->setState(Hand::selected);
					while (botLeft->getUcat() < 1.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}

					botLeft->setState(Hand::attackingForward);
					while (botLeft->getUcat() < 1.5f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botLeft->attack(*playerRight);

					// Reset to deselected after
					while (botLeft->getUcat() < 3.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botLeft->setState(Hand::deselected);
				}
				if (botMove == "bump") {
					botRight->setState(Hand::bumping);
					botLeft->setState(Hand::bumping);
					if (botLeft->getValue() > 0) {
						int newVal = botLeft->getValue() / 2;
						for (int i = 0; i < newVal; i++) {
							botLeft->countDown();
							botRight->countUp();
						}
					}
					else /* playerRight > 0 */ {
						int newVal = (int)(botRight->getValue() / 2);
						for (int i = 0; i < newVal; i++) {
							botRight->countDown();
							botLeft->countUp();
						}
					}

					// Reset to deselected after
					while (botRight->getUcat() < 2.0f && !glfwWindowShouldClose(window)) {
						if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
							// Render scene.
							render();
							// Swap front and back buffers.
							glfwSwapBuffers(window);
						}
					}
					botLeft->setState(Hand::deselected);
					botRight->setState(Hand::deselected);
				}
				 
			} else {
				gameOver = true;
				//cout << "Congrats, you won!" << endl;
			}
			
			if ((playerLeft->getValue() > 0) || (playerRight->getValue() > 0)){
				playerTurn = true;
			} else {
				gameOver = true;
				//cout << "Oh no, you lost!" << endl;
			}
		}
	} else if ((key == (unsigned int)' ') && gameOver == true) {
		//cout << "restarting" << endl;
		restart();
		gameOver = false;
		return;
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if (!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(1920, 1080, "Sticks", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
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
		if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
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
