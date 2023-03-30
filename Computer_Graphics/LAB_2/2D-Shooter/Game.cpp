#include "Game.h"

Game::Game(int width, int height) : mWindow(nullptr), mTicksCount(0), mIsRunning(true) {
	mWidth = width;
	mHeight = height;
	mActor = new Actor();
}

bool Game::Initialize() {
	if(!glfwInit()) {
		std::cout << "Error glfwInit()" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	mWindow = glfwCreateWindow(mWidth, mHeight, "2D Shooter", nullptr, nullptr);

	if(!mWindow) {
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(mWindow);
	glfwSwapInterval(1);
	//glViewport(0, 0, mWidth, mHeight);
	// framebuffer callback

	glewExperimental = true;

	if(glewInit() != GLEW_OK) {
		std::cout << "Error" << std::endl;
		return false;
	}

	mActor->Init();

	return true;
}

void Game::RunLoop() {

	while(mIsRunning) { 
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown() {
	delete mActor;
	glfwSetWindowShouldClose(mWindow, mIsRunning);
	glfwTerminate();
}

void Game::ProcessInput() {
	
	if(glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		mIsRunning = false;
	}
}

void Game::UpdateGame() {
	
}

void Game::GenerateOutput() {
	mActor->Draw();
	
	glfwPollEvents();
	glfwSwapBuffers(mWindow);
}
