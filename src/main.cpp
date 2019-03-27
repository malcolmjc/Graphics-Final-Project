/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include "sound.h"
#include <iostream>
#include <fstream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include <random>
#include "Program.h"
#include "MatrixStack.h"
#include <algorithm>
#include <ctime>
#include <chrono>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <mmsystem.h>

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;
shared_ptr<Shape> sphere, cobblestone, key, chest, wolf;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

class Square {
public:
	bool top, bottom, right, left;
	bool visited;

	Square() {
		top = true, bottom = true, right = true, left = true;
		visited = false;
	}
};

class Pair {
public:
	int x, y;
	Pair(int x, int y) {
		this->x = x;
		this->y = y;
	}
};

music_ music;
vector<Pair> movements;
class Wolf {
	public:
		int x, y, track1;
		int moveCount;
		Wolf() {
			x = -1; y = -1;
			moveCount = 1;
			track1 = music.init_music("../resources/howl.wav");
		}

		void move() {
			Pair first = movements.front();
			x = first.x;
			y = first.y;
			movements.erase(movements.begin());
			if (moveCount % 4 == 0) {
				music.play(track1);
				moveCount = 1;
			}
			moveCount++;
		}
};

Square squares[20][20];
bool editing = false;
bool returningToChest = true;
class camera
{
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
public:
	bool trippy;
	float yangle = M_PI / 2;
	glm::vec3 pos, rot;
	int w, a, s, d, h, j;
	int xPos = 0, yPos = 19;
	int direction = UP;
	int dIterations = 0, aIterations = 0;
	bool movingF = false, movingB = false;
	int speedIterations = 0;
	camera()
	{
		trippy = false;
		w = a = s = d = h = j = 0;
		pos = glm::vec3(0.0, 0.0, 3.0);
		rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (d == 1) {
			dIterations++;
			yangle = (M_PI / 24.0);
			if (dIterations == 12) {
				d = 0;
				dIterations = 0;
			}
		}

		else if (a == 1) {
			aIterations++;
			yangle = -(M_PI / 24.0);
			if (aIterations == 12) {
				a = 0;
				aIterations = 0;
			}
		}

		//always keep rot.y within range -2pi to 2pi
		rot.y += yangle;
		if (abs(rot.y - 2 * M_PI) < 0.01)
			rot.y = 0.0;
		else if (abs(rot.y + 2 * M_PI) < 0.01)
			rot.y = 0.0;

		if (movingF) {
			speed = 1.0;
			speedIterations++;
		}

		else if (movingB) {
			speed = -1.0;
			speedIterations++;
		}

		if (speedIterations == 6) {
			speed = 0.0;
			speedIterations = 0;
			movingF = false;
			movingB = false;
		}

		if (w == 1) {
			w = 0;
			if (abs(rot.y - 0) < 0.01) { //moving left one tile
				direction = LEFT;
				if (!squares[yPos][xPos].left && xPos > 0) {
					xPos--;
					speed = 1.0;
					movingF = true;
					cout << "moving left" << endl;
				}
			}

			else if ((abs(rot.y - M_PI / 2) < 0.01 || abs(rot.y + (3 * M_PI) / 2) < 0.01) && yPos > 0) { //moving up one tile
				direction = UP;
				if (!squares[yPos][xPos].top) {
					yPos--;
					speed = 1.0;
					movingF = true;
					cout << "moving up" << endl;
				}
			}

			else if ((abs(rot.y - M_PI) < 0.01 || abs(rot.y + M_PI) < 0.01) && xPos < 19) { //moving right one tile
				direction = RIGHT;
				if (!squares[yPos][xPos].right) {
					xPos++;
					speed = 1.0;
					movingF = true;
					cout << "moving right" << endl;
				}
			}

			else if ((abs(rot.y - (M_PI * 3) / 2) < 0.01 || abs(rot.y + M_PI / 2) < 0.01) && yPos < 19) { //moving down one tile
				direction = DOWN;
				if (!squares[yPos][xPos].bottom) {
					yPos++;
					speed = 1.0;
					movingF = true;
					cout << "moving down" << endl;
				}
			}
		}

		if (s == 1) {
			s = 0;
			if (abs(rot.y - 0) < 0.01 && xPos < 19) { //moving right one tile
				direction = RIGHT;
				if (!squares[yPos][xPos].right) {
					xPos++;
					speed = -1.0;
					movingB = true;
					cout << "moving right" << endl;
				}
			}

			else if ((abs(rot.y - M_PI / 2) < 0.01 || abs(rot.y + (3 * M_PI) / 2) < 0.01) && yPos < 19) { //moving down one tile
				direction = DOWN;
				if (!squares[yPos][xPos].bottom) {
					yPos++;
					speed = -1.0;
					movingB = true;
					cout << "moving down" << endl;
				}
			}

			else if ((abs(rot.y - M_PI) < 0.01 || abs(rot.y + M_PI) < 0.01) && xPos > 0) { //moving left one tile
				direction = LEFT;
				if (!squares[yPos][xPos].left) {
					xPos--;
					speed = -1.0;
					movingB = true;
					cout << "moving left" << endl;
				}
			}

			else if ((abs(rot.y - (M_PI * 3) / 2) < 0.01 || abs(rot.y + M_PI / 2) < 0.01) && yPos > 0) { //moving up one tile
				direction = UP;
				if (!squares[yPos][xPos].top) {
					yPos--;
					speed = -1.0;
					movingB = true;
					cout << "moving up" << endl;
				}
			}
		}

		squares[yPos][xPos].visited = true; // visit the square that you're at
		int index = 0;
		for (Pair p : movements) {
			if (p.x == xPos && p.y == yPos) {
				movements.erase(movements.begin() + index, movements.end());
			}
			index++;
		}
		movements.push_back(Pair(xPos, yPos));

		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;

		pos += glm::vec3(dir.x, dir.y, dir.z);

		if (h == 1)
			pos.y--;
		if (j == 1)
			pos.y++;

		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		glm::mat4 R2 = glm::rotate(glm::mat4(1), rot.z, glm::vec3(1, 0, 0));

		yangle = 0; //reset yangle
		return R2 * R * T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	int initMusic, keyMusic, endMusic;
	// Our shader program
	std::shared_ptr<Program> prog, psky, pwalls, pwalls2;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox, InstanceBuffer, InstanceBuffer2;

	//texture data
	GLuint FloorTexture;
	GLuint SkyBoxTextureDark, SkyBoxTextureLight, WallTexture;
	GLuint StoneTexture, GrassTexture, MoonTexture, SunTexture, GoldTexture, ChestTexture, WolfTexture;
	GLuint TrippyTexture1, TrippyTexture2, TrippyTexture3, TrippyTexture4, TrippyTexture5, TrippyTexture6,
		TrippyTexture7, TrippyTexture8;

	GLuint TrippyTextures[8];

	void emptySquares() {
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				squares[i][j] = Square();
				squares[i][j].bottom = false;
				squares[i][j].top = false;
				squares[i][j].right = false;
				squares[i][j].left = false;
			}
		}
	}

	void activateEditMode() {
		editing = true;
		mycam.rot.z = M_PI / 2;
		mycam.pos.y = -10.0;
		emptySquares();
	}

	void deactivateEditMode() {
		mycam.rot.z = 0;
		mycam.pos.y = 0;
		editing = false;
	}

	void resetCharacter() {
		if (!keyPickedUp) {
			mycam.pos.x = 0;
			mycam.pos.z = 3.0;
			mycam.xPos = 0;
			mycam.yPos = 19;
		}
		else {
			mycam.pos.x = -(6.0 * 19);
			mycam.pos.z = 3.0 - 6.0 * 19;
			mycam.xPos = 19;
			mycam.yPos = 0;
		}

		movements.clear();
		aiWolf = Wolf();
		deactivateEditMode();
	}

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			if (!mycam.movingB && !mycam.movingF)
				mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			if (!mycam.movingB && !mycam.movingF)
				mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_H && action == GLFW_PRESS)
		{
			mycam.h = 1;
		}
		if (key == GLFW_KEY_H && action == GLFW_RELEASE)
		{
			mycam.h = 0;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			if (mycam.xPos == 19 && mycam.yPos == 0) {
				if (!editing)
					pickUpKey();
				else {
					editing = false;
					returningToChest = true;
				}
			}
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			if (mycam.xPos == 0 && mycam.yPos == 19) {
				if (keyPickedUp)
					openChest();
				else
					resetCharacter();
			}
		}

		if (key == GLFW_KEY_G && action == GLFW_PRESS)
		{
			if (!editing)
				activateEditMode();
			else
				deactivateEditMode();
		}
		if (key == GLFW_KEY_J && action == GLFW_PRESS)
		{
			mycam.j = 1;
		}
		if (key == GLFW_KEY_J && action == GLFW_RELEASE)
		{
			mycam.j = 0;
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			if (editing) {
				if (squares[mycam.yPos][mycam.xPos].top == true)
					squares[mycam.yPos][mycam.xPos].top = false;
				else
					squares[mycam.yPos][mycam.xPos].top = true;
			}
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			if (editing) {
				if (squares[mycam.yPos][mycam.xPos].bottom == true)
					squares[mycam.yPos][mycam.xPos].bottom = false;
				else
					squares[mycam.yPos][mycam.xPos].bottom = true;
			}
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			if (editing) {
				if (squares[mycam.yPos][mycam.xPos].left == true)
					squares[mycam.yPos][mycam.xPos].left = false;
				else
					squares[mycam.yPos][mycam.xPos].left = true;
			}
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			if (editing) {
				if (squares[mycam.yPos][mycam.xPos].right == true)
					squares[mycam.yPos][mycam.xPos].right = false;
				else
					squares[mycam.yPos][mycam.xPos].right = true;
			}
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		/*double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}*/
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	int x, y;
	void makeMaze() {
		x = 0; y = 19;
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				squares[i][j] = Square();
			}
		}

		srand(time(NULL));
		vector<Pair> choices;
		int choice, spaces;
		while (x != 19 || y != 0) {
			choice = rand() % 4;
			spaces = rand() % 5 + 1;
			if (choice == 0)
				moveLeft(spaces);
			else if (choice == 1)
				moveRight(spaces);
			else if (choice == 2)
				moveDown(spaces);
			else
				moveUp(spaces);

			choices.push_back(Pair(x, y));
		}
	}

	void moveRight(int spaces) {
		while (x < 19 && spaces-- > 0) {
			squares[y][x].right = false;
			squares[y][x + 1].left = false;
			x++;
		}
	}

	void moveLeft(int spaces) {
		while (x > 0 && spaces-- > 0) {
			squares[y][x].left = false;
			squares[y][x - 1].right = false;
			x--;
		}
	}

	void moveDown(int spaces) {
		while (y < 19 && spaces-- > 0) {
			squares[y][x].bottom = false;
			squares[y + 1][x].top = false;
			y++;
		}
	}

	void moveUp(int spaces) {
		while (y > 0 && spaces-- > 0) {
			squares[y][x].top = false;
			squares[y - 1][x].bottom = false;
			y--;
		}
	}

	Wolf aiWolf;
	double startTime;
	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
	    aiWolf = Wolf();

		string resourceDirectory = "../resources";
		// Initialize mesh.
		sphere = make_shared<Shape>();
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();

		cobblestone = make_shared<Shape>();
		cobblestone->loadMesh(resourceDirectory + "/CobbleStones2.obj");
		cobblestone->resize();
		cobblestone->init();

		key = make_shared<Shape>();
		key->loadMesh(resourceDirectory + "/key.obj");
		key->resize();
		key->init();

		chest = make_shared<Shape>();
		chest->loadMesh(resourceDirectory + "/treasurechest.obj");
		chest->resize();
		chest->init();

		wolf = make_shared<Shape>();
		wolf->loadMesh(resourceDirectory + "/Wolf_obj.obj");
		wolf->resize();
		wolf->init();

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,//LD
			1.0, -1.0,  1.0,//RD
			1.0,  1.0,  1.0,//RU
			-1.0,  1.0,  1.0,//LU
		};
		//make it a bit smaller
		for (int i = 0; i < 12; i++)
			cube_vertices[i] *= 0.5;
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		GLfloat cube_norm[] = {
			// front colors
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
		};
		glGenBuffers(1, &VertexNormDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//color
		glm::vec2 cube_tex[] = {
			// front colors
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(0.0, 0.0),

		};
		glGenBuffers(1, &VertexTexBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

		// INSTANCE BUFFER STUFF \\
		//generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
		glm::vec4 *positions = new glm::vec4[400];
		float z = 0.0, x = 0;

		makeMaze();

		int count = 0;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				positions[count++] = glm::vec4(x, 0, z, 0);
				x -= 6.0;
			}

			x = 0.0;
			z -= 6.0;
		}

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, (400) * sizeof(glm::vec4), positions, GL_STATIC_DRAW);
		int position_loc = glGetAttribLocation(pwalls->pid, "InstancePos");
		for (int i = 0; i < 400; i++)
		{
			// Set up the vertex attribute
			glVertexAttribPointer(position_loc + i,              // Location
				4, GL_FLOAT, GL_FALSE,       // vec4
				sizeof(vec4),                // Stride
				(void *)(sizeof(vec4) * i)); // Start offset
											 // Enable it
			glEnableVertexAttribArray(position_loc + i);
			// Make it instanced
			glVertexAttribDivisor(position_loc + i, 1);
		}

		/*//pwalls 2

		//generate vertex buffer to hand off to OGL ###########################
		glGenBuffers(1, &InstanceBuffer2);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer2);
		z = 0.0 ; x = 0.0;
		glm::vec4 *positions2 = new glm::vec4[400];

		count = 0;
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				if (squares[i][j].left == true)
					positions2[count++] = glm::vec4(x, 0, z, 0);

				else {
					cout << "else" << endl;
					positions2[count++] = glm::vec4(100000, 0, 0, 0);
				}
				x -= 2;
			}

			x = 0;
			z -= 2;
		}

		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, (400) * sizeof(glm::vec4), positions2, GL_STATIC_DRAW);
		position_loc = glGetAttribLocation(pwalls2->pid, "InstancePos2");
		for (int i = 0; i < 400; i++)
		{
			// Set up the vertex attribute
			glVertexAttribPointer(position_loc + i,              // Location
				4, GL_FLOAT, GL_FALSE,       // vec4
				sizeof(vec4),                // Stride
				(void *)(sizeof(vec4) * i)); // Start offset
											 // Enable it
			glEnableVertexAttribArray(position_loc + i);
			// Make it instanced
			glVertexAttribDivisor(position_loc + i, 1);
		}*/

		glBindVertexArray(0);

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/brick.png";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &FloorTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FloorTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 2
		str = resourceDirectory + "/stars_milky_way.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SkyBoxTextureDark);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SkyBoxTextureDark);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 2
		str = resourceDirectory + "/digital.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &WallTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, WallTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SkyBoxTextureLight);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SkyBoxTextureLight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/grass.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &GrassTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GrassTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/moon.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &MoonTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, MoonTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/sun.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &SunTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, SunTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/gold.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &GoldTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GoldTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/treasurechest.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &ChestTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ChestTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/Wolf_Body.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &WolfTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, WolfTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		str = resourceDirectory + "/trippy1.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[0] = TrippyTexture1;

		str = resourceDirectory + "/trippy2.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[1] = TrippyTexture2;

		str = resourceDirectory + "/trippy3.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture3);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[2] = TrippyTexture3;

		str = resourceDirectory + "/trippy4.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture4);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[3] = TrippyTexture4;

		str = resourceDirectory + "/trippy5.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture5);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture5);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[4] = TrippyTexture5;

		str = resourceDirectory + "/trippy6.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture6);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture6);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[5] = TrippyTexture6;

		str = resourceDirectory + "/trippy7.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture7);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture7);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[6] = TrippyTexture7;

		str = resourceDirectory + "/trippy8.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &TrippyTexture8);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TrippyTexture8);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		TrippyTextures[7] = TrippyTexture8;

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		GLuint Tex3Location = glGetUniformLocation(pwalls->pid, "texwall");
		GLuint Tex4Location = glGetUniformLocation(pwalls2->pid, "texwall2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		glUseProgram(pwalls->pid);
		glUniform1i(Tex3Location, 0);

		glUseProgram(pwalls2->pid);
		glUniform1i(Tex4Location, 0);


		startTime = glfwGetTime();
		music.play(initMusic);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		initMusic = music.init_music("../resources/midnas_lament.mp3");
		keyMusic = music.init_music("../resources/mm.mp3");
		endMusic = music.init_music("../resources/song_of_storms.mp3");
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addUniform("floor");
		prog->addUniform("highlighted");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");


		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		pwalls = std::make_shared<Program>();
		pwalls->setVerbose(true);
		pwalls->setShaderNames(resourceDirectory + "/wallvertex.glsl", resourceDirectory + "/wallfrag.glsl");
		if (!pwalls->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pwalls->addUniform("P");
		pwalls->addUniform("V");
		pwalls->addUniform("M");
		pwalls->addUniform("campos");
		pwalls->addUniform("xOffset");
		pwalls->addUniform("zOffset");

		pwalls->addAttribute("vertPos");
		pwalls->addAttribute("vertNor");
		pwalls->addAttribute("vertTex");
		pwalls->addAttribute("InstancePos");

		pwalls2 = std::make_shared<Program>();
		pwalls2->setVerbose(true);
		pwalls2->setShaderNames(resourceDirectory + "/wallvertex2.glsl", resourceDirectory + "/wallfrag2.glsl");
		if (!pwalls2->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pwalls2->addUniform("P");
		pwalls2->addUniform("V");
		pwalls2->addUniform("M");
		pwalls2->addUniform("campos");

		pwalls2->addAttribute("vertPos");
		pwalls2->addAttribute("vertNor");
		pwalls2->addAttribute("vertTex");
		pwalls2->addAttribute("InstancePos2");
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	mat4 FarLeft;
	glm::mat4 FarM;
	glm::mat4 OrigM;
	int gridSizeX, gridSizeY;

	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}

		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		mat4 M_1 = TransSky * RotateXSky;
		M = TransSky * RotateXSky * SSky;

		drawSky(P, V, M);

		glm::mat4 SMoon = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		glm::mat4 TransMoon = translate(glm::mat4(1.0f), glm::vec3(-200.0f, 99.2f, -200.0f));
		glm::mat4 M_2 = TransMoon * M_1;
		M = TransMoon * SMoon;

		drawMoon(P, V, M);

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = -3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.2f, -3 + trans));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

		M = TransZ;

		drawFloors(P, V, M);

		if (!chestOpened)
			drawWalls(P, V, M);
	}

	void drawSky(mat4 P, mat4 V, mat4 M) {
		// Draw the box using GLSL.
		psky->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		if (!chestOpened)
			glBindTexture(GL_TEXTURE_2D, SkyBoxTextureDark);
		else
			glBindTexture(GL_TEXTURE_2D, SkyBoxTextureLight);

		glDisable(GL_DEPTH_TEST);
		sphere->draw(psky, FALSE);
		glEnable(GL_DEPTH_TEST);

		psky->unbind();
	}

	void drawMoon(mat4 P, mat4 V, mat4 M) {
		// Draw the box using GLSL.
		psky->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		if (!chestOpened)
			glBindTexture(GL_TEXTURE_2D, MoonTexture);
		else
			glBindTexture(GL_TEXTURE_2D, SunTexture);

		sphere->draw(psky, FALSE);

		psky->unbind();
	}

	bool keyPickedUp, chestOpened = false;
	vector<mat4> wallLocs;
	glm::mat4 firstDrawn;
	void drawFloors(mat4 P, mat4 V, mat4 M) {
		// Draw the box using GLSL.
		prog->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		if (!chestOpened)
			glBindTexture(GL_TEXTURE_2D, FloorTexture);
		else
			glBindTexture(GL_TEXTURE_2D, GrassTexture);

		gridSizeX = 20;
		gridSizeY = 20;

		glm::mat4 TransX = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		OrigM = M;
		glm::mat4 TransZ;
		glUniform1f(prog->getUniform("floor"), 1.0);
		for (int i = 0; i < 20; i++) {
			TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 6.0f*i));
			M = OrigM * TransZ;
			TransX = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));;
			for (int j = 0; j < 20; j++) {
				M = M * TransX;
				if (i == 0 && j == 0) {
					firstDrawn = M;
				}
				TransX = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.0f, 0.0f));
				glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
				mat4 M_DRAW = M * S;
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M_DRAW[0][0]);
				glUniform1f(prog->getUniform("highlighted"), 0.0);
				if (editing && i == mycam.xPos && j == abs(mycam.yPos - 19)) {
					glUniform1f(prog->getUniform("highlighted"), 1.0);
				}
				cobblestone->draw(prog, FALSE);

				FarLeft = M;
			}
		}

		FarM = M;

		glUniform1f(prog->getUniform("highlighted"), 0.0);
		mat4 TransY, S;
		if (!keyPickedUp) {
			S = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
			TransY = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.6f, 1.5f));
			mat4 PrevM = M * TransY;
			M = M * TransY * S;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform1f(prog->getUniform("floor"), 0.0);
			glBindTexture(GL_TEXTURE_2D, GoldTexture);
			key->draw(prog, false);
		}

		if (!chestOpened) {
			TransY = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.0f));
			S = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
			M = firstDrawn * TransY * S;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glBindTexture(GL_TEXTURE_2D, ChestTexture);
			chest->draw(prog, false);
		}

		//wolf stuff here
		if (!editing && !chestOpened) {
			if (glfwGetTime() - startTime > 2.0) {
				aiWolf.move();
				startTime = glfwGetTime();
			}

			if (aiWolf.x == mycam.xPos && aiWolf.y == mycam.yPos)
				resetCharacter();
			
			glm::mat4 TransZOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 6.0f*aiWolf.x));
			glm::mat4 TransXOffset = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f*(19 - aiWolf.y), 0.0f, 0.0f));
			mat4 Vi = glm::transpose(V);
			Vi[0][3] = 0;
			Vi[1][3] = 0;
			Vi[2][3] = 0;
			M = M * TransXOffset * TransZOffset * Vi;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glBindTexture(GL_TEXTURE_2D, WolfTexture);
			wolf->draw(prog, false);
		}

		glBindVertexArray(0);

		prog->unbind();
	}

	void pickUpKey() {
		keyPickedUp = true;
		trippy = true;
		mycam.trippy = true;
		movements.clear();
		aiWolf = Wolf();

		music.fade_out(initMusic, 10);
		music.fade_in_and_play(keyMusic, 1100);
		makeMaze();
	}

	void openChest() {
		chestOpened = true;
		trippy = false;
		mycam.trippy = false;
		music.fade_out(keyMusic, 500);
		music.fade_in_and_play(endMusic, 600);
	}

	bool trippy = false;
	int trippyIterations = 0;
	int lastChoice=0;
	void drawWalls(mat4 P, mat4 V, mat4 M) {
		pwalls->bind();

		glUniformMatrix4fv(pwalls->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pwalls->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pwalls->getUniform("campos"), 1, &mycam.pos[0]);

		glActiveTexture(GL_TEXTURE0);
		if (trippy) {
			if (trippyIterations++ == 8) {
				lastChoice = TrippyTextures[rand() % 8];
				trippyIterations = 0;
			}
			glBindTexture(GL_TEXTURE_2D, lastChoice);
		}
		else
			glBindTexture(GL_TEXTURE_2D, WallTexture);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);

		mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
		OrigM = OrigM * TransZ;

		mat4 rotateY = glm::rotate(glm::mat4(1), ((float)M_PI) / 2, glm::vec3(0, 1, 0));
		glm::mat4 TransYRight = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.3f, 3.0f));
		glm::mat4 TransYLeft = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.3f, -3.0f));
		mat4 TransYUp = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 1.3f, 0.0f));
		mat4 TransYDown = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 1.3f, 0.0f));

		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(6.0f, 2.9f, 0.0f));
		glEnable(GL_DEPTH_TEST);

		glm::mat4 TransXOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 TransZOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		for (int x = 0; x < 20; x++) {
			glm::mat4 TransZOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 6.0f*x));
			for (int y = 19; y >= 0; y--) {
				glm::mat4 TransXOffset = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f*(19 - y), 0.0f, 0.0f));
				if (squares[y][x].visited == true) {
					//draw right wall
					if (squares[y][x].right) {
						M = OrigM * TransXOffset * TransZOffset * TransYRight * S;
						glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
					}

					//draw left wall
					if (squares[y][x].left) {
						M = OrigM * TransXOffset * TransZOffset * TransYLeft * S;
						glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
					}

					//draw bottom wall
					if (squares[y][x].bottom) {
						M = OrigM * TransXOffset * TransZOffset * TransYDown * rotateY * S;
						glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
					}

					//draw top wall
					if (squares[y][x].top) {
						M = OrigM * TransXOffset * TransZOffset * TransYUp * rotateY * S;
						glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
						glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
					}
				}
			}
		}

		pwalls->unbind();


		/*glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.3f, 3.0f));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(6.0f, 2.9f, 0.0f));
		mat4 M_1 = FarLeft * TransY;
		M = FarLeft*TransY*S;
		glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		glEnable(GL_DEPTH_TEST);
		//DRAW Right Walls
		// ***************************************************
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 400);

		TransY = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.3f, -3.0f));
		M = FarLeft*TransY*S;
		glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//DRAW Left Walls
		// ***************************************************
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 400);

		mat4 rotateY = glm::rotate(glm::mat4(1), ((float)M_PI) / 2, glm::vec3(0, 1, 0));
		TransY = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 1.3f, 0.0f));
		M = FarLeft * TransY * rotateY * S;
		glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//DRAW Top Walls
		// ***************************************************
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 400);

		TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 1.3f, 0.0f));
		M = FarLeft * TransY * rotateY * S;
		glUniformMatrix4fv(pwalls->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//DRAW Bottom Walls
		// ***************************************************
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 400);

		glBindVertexArray(0);

		pwalls->unbind();

		pwalls2->bind();

		glUniformMatrix4fv(pwalls2->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pwalls2->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pwalls2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pwalls2->getUniform("campos"), 1, &mycam.pos[0]);

		glBindVertexArray(VertexArrayID);
		//actually draw from vertex 0, 3 vertices

		rotateY = glm::rotate(glm::mat4(1), ((float)M_PI)/2, glm::vec3(0, 1, 0));
		glm::mat4 TransX = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.5f));
		M = M_1 * TransX * rotateY * S;
		glUniformMatrix4fv(pwalls2->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, 400);

		glBindVertexArray(0);

		pwalls2->unbind();*/


	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
		// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
