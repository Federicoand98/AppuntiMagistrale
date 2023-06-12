#include "Game.h"

Game::Game(int width, int height) : mWindow(nullptr), mIsRunning(true), mDeltaTime(0), mLastFrame(0), mNumEnemies(10), mNumEnemiesKilled(0) {
	mWidth = width;
	mHeight = height;

	mBackground = new Background(this);

	mActor = new Actor(this, 40.0, 40.0);

	for(int i = 0; i < mNumEnemies; i++) {
		mEnemies.push_back(new Enemy(this, 20.0, 20.0));
	}

	for(int i = 1; i <= mActor->GetLives(); i++) {
		float x = 45 * (float)i;
		float y = mHeight - 45;
		mHearth.push_back(new Hearth(this, 15, 15, {x, y}));
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

	mBackground->Init(mActor->GetLives());

	for(Enemy* e : mEnemies) {
		e->Init();
	}

	for(Hearth* h : mHearth) {
		h->Init();
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
	delete mBackground;
	delete mActor;

	for(Enemy* e : mEnemies) {
		delete e;
	}

	for (Hearth* h : mHearth) {
		delete h;
	}

	for(Skull* s : mSkulls) {
		delete s;
	}

	mSkulls.clear();
	mEnemies.clear();
	mHearth.clear();

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

	mBackground->Update(mDeltaTime);

	mActor->Update(mDeltaTime);

	for(Enemy* e : mEnemies) {
		e->Update(mDeltaTime);
	}

	for(Skull* s : mSkulls) {
		s->Update(mDeltaTime);
	}

	if(mActor->GetProjectiles().size() > 0) {
		for(Projectile* proj : mActor->GetProjectiles()) {
			proj->Update(mDeltaTime);
		}
	}

	// collision detection
	bool collided = false;

	// if an enemy touch the player -> Take damage
	for (int i = 0; i < mEnemies.size() && !mActor->IsInvincible() && !collided; i++) {
		float dist = distance(mActor->GetTransform(), mEnemies.at(i)->GetTransform());

		if (dist < mActor->GetWidth() + mEnemies.at(i)->GetWidth()) {
			collided = true;

			delete mHearth.at(mActor->GetLives() - 1);
			mHearth.pop_back();
			mActor->TakeDamage();
		}
	}

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

					float skullX = mWidth - 60;
					float skullY = 60 * (mSkulls.size() + 1);
					Skull* skull = new Skull(this, 15, 15, { skullX, skullY });
					skull->Init();
					mSkulls.push_back(skull);
					std::cout << mSkulls.size() << std::endl;
				}
			}
		}
	}

	// terminazione
	if(mEnemies.empty()) {
		mIsRunning = false;
	}

	if(mActor->GetLives() == 0 && mHearth.empty()) {
		mIsRunning = false;
	}
}

void Game::GenerateOutput() {
	glClear(GL_COLOR_BUFFER_BIT);

	mBackground->Draw();

	mActor->Draw();

	for(Enemy* e : mEnemies) {
		e->Draw();
	}

	for (Hearth* h : mHearth) {
		h->Draw();
	}

	for(Skull* s : mSkulls) {
		s->Draw();
	}

	if(mActor->GetProjectiles().size() > 0) {
		for(Projectile* proj : mActor->GetProjectiles()) {
			proj->Draw();
		}
	}
	
	glfwPollEvents();
	glfwSwapBuffers(mWindow);
}


