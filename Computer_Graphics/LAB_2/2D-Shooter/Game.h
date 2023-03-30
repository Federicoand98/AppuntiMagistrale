#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Actor.h"

struct Vector2 {
	float x;
	float y;
};

class Game {

public:
	Game(int width, int height);

	// Initialize the game
	bool Initialize();

	// Runs the game loop until the game is over
	void RunLoop();

	// Shutdown the game
	void Shutdown();

private:
	// Helper functions for the game loop
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	// Game should continue to run
	bool mIsRunning;
	// Number of ticks since start of the game
	uint32_t mTicksCount;
	// Window
	GLFWwindow* mWindow;
	// Width and Height
	int mWidth;
	int mHeight;

	class Actor* mActor;
};

