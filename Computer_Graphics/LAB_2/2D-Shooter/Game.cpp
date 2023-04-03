#include "Game.h"

Game::Game(int width, int height) : mWindow(nullptr), mIsRunning(true), mDeltaTime(0), mLastFrame(0), mNumEnemies(10) {
	mWidth = width;
	mHeight = height;
	mActor = new Actor(this, 40.0, 40.0);

	for(int i = 0; i < mNumEnemies; i++) {
		mEnemies.push_back(new Enemy(this, 20.0, 20.0));
	}
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
	glViewport(0, 0, mWidth, mHeight);
	// framebuffer callback

	glewExperimental = true;

	if(glewInit() != GLEW_OK) {
		std::cout << "Error" << std::endl;
		return false;
	}

	//InitBackground();

	for(Enemy* e : mEnemies) {
		e->Init();
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

	for(Enemy* e : mEnemies) {
		delete e;
	}

	mEnemies.clear();

	glfwSetWindowShouldClose(mWindow, mIsRunning);
	glfwTerminate();
}

void Game::ProcessInput() {
	
	if(glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		mIsRunning = false;
	}

	mActor->ProcessKeyboardInput(mWindow);
}

void Game::UpdateGame() {
	float currentFrame = glfwGetTime();
	mDeltaTime = currentFrame - mLastFrame;
	mLastFrame = currentFrame;

	mActor->Update(mDeltaTime);

	for(Enemy* e : mEnemies) {
		e->Update(mDeltaTime);
	}


	if(mActor->GetProjectiles().size() > 0) {
		for(Projectile* proj : mActor->GetProjectiles()) {
			proj->Update(mDeltaTime);
		}
	}

	// collision detection
	bool collided = false;

	// if an enemy touch the player -> GAME OVER
	/*
	for (int i = 0; i < mEnemies.size() && !collided; i++) {
		float dist = distance(mActor->GetTransform(), mEnemies.at(i)->GetTransform());

		if (dist < mActor->GetWidth() + mEnemies.at(i)->GetWidth()) {
			collided = true;

			mIsRunning = false;
		}
	}
	*/

	collided = false;

	// if a projectile hit one enemy -> enemy dies
	if(mActor->GetProjectiles().size() > 0) {
		
		for(int i = 0; i < mEnemies.size() && !collided; i++) {
			for(int j = 0; j < mActor->GetProjectiles().size() && !collided; j++) {
				float dist = distance(mEnemies.at(i)->GetTransform(), mActor->GetProjectiles().at(j)->GetTransform());

				if(dist < mEnemies.at(i)->GetWidth() + mActor->GetProjectiles().at(j)->GetWidth()) {
					collided = true;

					delete mEnemies.at(i);
					mEnemies.erase(mEnemies.begin() + i);
				}
			}
		}
	}

	// terminazione
	if(mEnemies.empty()) {
		mIsRunning = false;
	}
}

void Game::GenerateOutput() {
	glClear(GL_COLOR_BUFFER_BIT);

	//DrawBackground();

	mActor->Draw();

	for(Enemy* e : mEnemies) {
		e->Draw();
	}

	if(mActor->GetProjectiles().size() > 0) {
		for(Projectile* proj : mActor->GetProjectiles()) {
			proj->Draw();
		}
	}
	
	glfwPollEvents();
	glfwSwapBuffers(mWindow);
}

void Game::InitBackground() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexBG.glsl";
	char* fragmentShader = (char*)"shaders/fragmentBG.glsl";

	mBackgroundShader = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mBackgroundShader);

	mPoints.push_back({ 1, 1 });
	mPoints.push_back({ -1, 1 });
	mPoints.push_back({ -1, -1 });
	mPoints.push_back({ -1, -1 });
	mPoints.push_back({ 1, -1 });
	mPoints.push_back({ 1, 1 });

	mTime = glGetUniformLocation(mBackgroundShader, "time");
	mRes = glGetUniformLocation(mBackgroundShader, "resolution");

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mPoints.size() * sizeof(Vector), &mPoints.at(0), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void Game::DrawBackground() {
	float time = static_cast<float>(glfwGetTime());
	glm::vec2 res = glm::vec2(mWidth, mHeight);

	glUniform1f(mTime, time);
	glUniformMatrix2fv(mRes, 1, GL_FALSE, glm::value_ptr(res));

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size());
}


