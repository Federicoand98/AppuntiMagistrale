#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Actor.h"
#include "Enemy.h"
#include "Projectile.h"

class Game {

public:
	Game(int width, int height);

	// Initialize the game
	bool Initialize();

	// Runs the game loop until the game is over
	void RunLoop();

	// Shutdown the game
	void Shutdown();

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }

private:
	// Helper functions for the game loop
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	void InitBackground();
	void DrawBackground();

	// Game should continue to run
	bool mIsRunning;
	// deltaTime
	float mDeltaTime;
	float mLastFrame;
	// Window
	GLFWwindow* mWindow;
	// Width and Height
	int mWidth;
	int mHeight;

	// background
	unsigned int mBackgroundShader;
	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int mTime;
	unsigned int mRes;
	std::vector<Vector> mPoints;

	// gameplay related things
	int mNumEnemies;

	class Actor* mActor;
	std::vector<class Enemy*> mEnemies;
};

