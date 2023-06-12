#include "Enemy.h"

#include "Game.h"

Enemy::Enemy(Game* game, float width, float height): mVAO(0), mVBO(0), mShaderId(0), mDirection({0, 0}), mSpeed(150) {
	std::random_device rd;
	std::mt19937 rng(rd());

	this->mGame = game;

	// spawn random
	float min = width * 2;
	float h = (float)game->GetHeight() - 2.0 * height;
	float w = (float)game->GetWidth() - 2.0 * width;

	std::uniform_real_distribution<float> uniH(min, h);
	std::uniform_real_distribution<float> uniW(min, w);

	float randX = uniW(rng);
	float randY = uniH(rng);

	this->mTransform = {
		randX,
		randY,
		1.0f,
		1.0f
	};

	this->mEnemyWidth = width;
	this->mEnemyHeight = height;

	this->mSeconds = time(nullptr);

	ChooseNewDirection();
}

Enemy::~Enemy() {
	mPoints.clear();

	glBindVertexArray(0);

	glDeleteProgram(mShaderId);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void Enemy::initShader() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	mShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mShaderId);
}

void Enemy::Init() {
	float step = PI / mNTriangles;
	int comp = 0;

	for (double i = 0; i < mNTriangles; i += step) {
		float x = cos(i * step);
		float y = sin(i * step);
		float x2 = cos((i + 1) * step);
		float y2 = sin((i + 1) * step);

		mPoints.push_back({ x, y, 0.0, 0.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 0.0, 0.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 0.0, 0.0, 1.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = 0.4 + cos(i * step) * 0.2;
		float y = 0.4 + sin(i * step) * 0.2;
		float x2 = 0.4 + cos((i + 1) * step) * 0.2;
		float y2 = 0.4 + sin((i + 1) * step) * 0.2;

		mPoints.push_back({ x, y, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 1.0, 1.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = -0.4 + cos(i * step) * 0.2;
		float y = 0.4 + sin(i * step) * 0.2;
		float x2 = -0.4 + cos((i + 1) * step) * 0.2;
		float y2 = 0.4 + sin((i + 1) * step) * 0.2;

		mPoints.push_back({ x, y, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 1.0, 1.0, 1.0 });
	}

	mPoints.push_back({ 1, 0, 0, 0, 1, 0 });
	mPoints.push_back({ -1, 0, 0, 0, 1, 0 });
	mPoints.push_back({ 1, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ 1, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ -1, 0, 0, 0, 1, 0 });
	mPoints.push_back({ -1, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ -1, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ -0.8, -1, 0, 0, 1, 0 });
	mPoints.push_back({ -0.6, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ -0.6, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ -0.4, -1, 0, 0, 1, 0 });
	mPoints.push_back({ -0.2, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ -0.2, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ 0, -1, 0, 0, 1, 0 });
	mPoints.push_back({ 0.2, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ 0.2, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ 0.4, -1, 0, 0, 1, 0 });
	mPoints.push_back({ 0.6, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ 0.6, -0.5, 0, 0, 1, 0 });
	mPoints.push_back({ 0.8, -1, 0, 0, 1, 0 });
	mPoints.push_back({ 1, -0.5, 0, 0, 1, 0 });

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

void Enemy::Draw() {
	glm::mat4 Model = glm::mat4(1.0);
	glm::mat4 Projection = glm::ortho(0.0f, 800.0f, 0.0f, 800.0f);

	Model = glm::translate(Model, glm::vec3(mTransform.xPos, mTransform.yPos, 0.0f));
	Model = glm::scale(Model, glm::vec3(mEnemyWidth * mTransform.xScale, mEnemyHeight * mTransform.yScale, 1.0f));

	glUniformMatrix4fv(mProjModel, 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(mMatModel, 1, GL_FALSE, glm::value_ptr(Model));

	glBindVertexArray(mVAO);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size());
}

void Enemy::Update(float deltaTime) {
	time_t currentSeconds = time(nullptr);

	if(currentSeconds - mSeconds > 10) {
		mSeconds = time(nullptr);

		ChooseNewDirection();
	}

	mTransform.xPos += mDirection.x * mSpeed * deltaTime;
	mTransform.yPos += mDirection.y * mSpeed * deltaTime;

	// check if the position is over the screen 
	if (mTransform.xPos > mGame->GetWidth() - mEnemyWidth || mTransform.xPos <= 0 + mEnemyWidth || mTransform.yPos > mGame->GetHeight() - mEnemyHeight|| mTransform.yPos <= 0 + mEnemyHeight) {
		//ChooseNewDirection();
		mDirection.x = -mDirection.x;
		mDirection.y = -mDirection.y;
	}
}

void Enemy::ChooseNewDirection() {
	float possibleValues[] = { 0, 1, -1 };

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(0, 2);

	mDirection.x = possibleValues[uni(rng)];
	mDirection.y = possibleValues[uni(rng)];
}

