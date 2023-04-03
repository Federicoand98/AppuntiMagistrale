#include "Actor.h"

#include "Game.h"

Actor::Actor(class Game* game, float width, float height) : mVAO(0), mVBO(0), mShaderId(0), mDirection({0, 0}), mSpeed(250), mRotation(20.0f), mShootingCooldown(0.3) {
	this->mGame = game;

	this->mTransform = {
		static_cast<float>(mGame->GetWidth()) / 2.0f,
		static_cast<float>(mGame->GetHeight()) / 2.0f,
		1.0f,
		1.0f
	};

	this->mActorWidth = width;
	this->mActorHeight = height;

	this->mSeconds = time(nullptr);
}

Actor::~Actor() {
	mPoints.clear();

	for(Projectile* p : mProjectiles) {
		delete p;
	}

	mProjectiles.clear();

	glBindVertexArray(0);

	glDeleteProgram(mShaderId);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void Actor::initShader() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	mShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mShaderId);
}

void Actor::Init() {
	float step = (1.7 * PI) / mNTriangles;
	int comp = 0;

	for(double i = 0; i < mNTriangles; i += step) {
		float x = cos(i * step);
		float y = sin(i * step);
		float x2 = cos((i + 1) * step);
		float y2 = sin((i + 1) * step);

		mPoints.push_back({ x, y, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 0.0, 0.0, 1.0 });
	}

	initShader();

	mMatModel = glGetUniformLocation(mShaderId, "Model");
	mProjModel = glGetUniformLocation(mShaderId, "Projection");

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mPoints.size() * sizeof(Point), &mPoints[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Actor::Draw() {
	glm::mat4 Model = glm::mat4(1.0);
	glm::mat4 Projection = glm::ortho(0.0f, (float)mGame->GetWidth(), 0.0f, (float)mGame->GetHeight());

	Model = glm::translate(Model, glm::vec3(mTransform.xPos, mTransform.yPos, 0.0f));
	Model = glm::rotate(Model, glm::radians(mRotation), glm::vec3(0.0f, 0.0f, 1.0f));
	Model = glm::scale(Model, glm::vec3(mActorWidth * mTransform.xScale, mActorHeight * mTransform.yScale, 1.0f));

	glUniformMatrix4fv(mProjModel, 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(mMatModel, 1, GL_FALSE, glm::value_ptr(Model));

	glBindVertexArray(mVAO);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size());
}

void Actor::Update(float deltaTime) {
	mTransform.xPos += mDirection.x * mSpeed * deltaTime;
	mTransform.yPos += mDirection.y * mSpeed * deltaTime;

	// check if the position is over the screen 
	if(mTransform.xPos > mGame->GetWidth()) {
		mTransform.xPos = mGame->GetWidth() - mActorWidth / 2;
	} else if(mTransform.xPos <= 0) {
		mTransform.xPos = mActorWidth / 2;
	} else if(mTransform.yPos > mGame->GetHeight()) {
		mTransform.yPos = mGame->GetHeight() - mActorHeight / 2;
	} else if(mTransform.yPos <= 0) {
		mTransform.yPos = mActorHeight / 2;
	}

	// check the time to live of every instantiated projectile
	time_t current = time(nullptr);

	for(int i = 0; i < mProjectiles.size(); i++) {
		if(current - mProjectiles.at(i)->GetTime() > mProjectiles.at(i)->GetTimeToLive()) {
			delete mProjectiles.at(i);
			mProjectiles.erase(mProjectiles.begin() + i);
		}
	}
}

void Actor::Shoot() {
	time_t currentSeconds = time(nullptr);

	if (currentSeconds - mSeconds > mShootingCooldown) {
		mSeconds = time(nullptr);

		int direction;

		if(mDirection.x > 0) {
			direction = RIGHT;
		} else if(mDirection.x < 0) {
			direction = LEFT;
		} else if(mDirection.y > 0) {
			direction = UP;
		} else if(mDirection.y < 0) {
			direction = DOWN;
		} else {
			direction = RIGHT;
		}

		Projectile* p = new Projectile(mGame, 15, 15, { mTransform.xPos, mTransform.yPos }, direction);

		p->Init();

		mProjectiles.push_back(p);
	}
}


void Actor::ProcessKeyboardInput(GLFWwindow* window) {
	mDirection = { 0, 0 };

	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		mDirection.x += 1;
		mRotation = 20.0f;
	}

	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		mDirection.x -= 1;
		mRotation = 200.0f;
	}

	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		mDirection.y += 1;
		mRotation = 110.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		mDirection.y -= 1;
		mRotation = 290.0f;
	}

	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		Shoot();
	}
}

