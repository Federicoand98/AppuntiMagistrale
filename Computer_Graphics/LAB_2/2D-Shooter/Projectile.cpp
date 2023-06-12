#include "Projectile.h"
#include "Game.h"

Projectile::Projectile(Game* game, float width, float height, Vector spawnPosition, int direction)
	: mHeight(height), mWidth(width), mVAO(0), mVBO(0), mSpeed(500), mShaderId(0), mSpawnPosition(spawnPosition), mRotation(0), mTimeToLive(5) {

	srand((unsigned)time(nullptr));

	mGame = game;

	this->mTransform = {
		spawnPosition.x,
		spawnPosition.y,
		1.0f,
		1.0f
	};

	switch (direction) {
	case UP:
		mDirection = { 0, 1 };
		break;
	case DOWN:
		mDirection = { 0, -1 };
		break;
	case RIGHT:
		mDirection = { 1, 0 };
		break;
	case LEFT:
		mDirection = { -1, 0 };
		break;
	default: ;
		mDirection = { 0, 0 };
		break;
	}

	this->mSeconds = time(nullptr);
}

Projectile::~Projectile() {
	mPoints.clear();

	glBindVertexArray(0);

	glDeleteProgram(mShaderId);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void Projectile::Init() {
	float step = (2 * PI) / mNTriangles;
	int comp = 0;

	for (double i = 0; i < mNTriangles; i += step) {
		float x = cos(i * step) * 0.9;
		float y = sin(i * step) * 0.9;
		float x2 = cos((i + 1) * step) * 0.9;
		float y2 = sin((i + 1) * step) * 0.9;

		mPoints.push_back({ x, y, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 1.0, 1.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = cos(i * step);
		float y = sin(i * step);
		float x2 = cos((i + 1) * step);
		float y2 = sin((i + 1) * step);

		mPoints.push_back({ x, y, 0.5, 0.0, 0.0, 0.5 });
		mPoints.push_back({ x2, y2, 0.5, 0.0, 0.0, 0.5 });
		mPoints.push_back({ 0, 0, 0.0, 0.0, 0.0, 0.5 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = -0.5 + cos(i * step) * 0.1;
		float y = 0.5 + sin(i * step) * 0.1;
		float x2 = -0.5 + cos((i + 1) * step) * 0.1;
		float y2 = 0.5 + sin((i + 1) * step) * 0.1;

		mPoints.push_back({ x, y, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 0.0, 0.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = 0.5 + cos(i * step) * 0.1;
		float y = 0.5 + sin(i * step) * 0.1;
		float x2 = 0.5 + cos((i + 1) * step) * 0.1;
		float y2 = 0.5 + sin((i + 1) * step) * 0.1;

		mPoints.push_back({ x, y, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 0.0, 0.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = -0.5 + cos(i * step) * 0.1;
		float y = -0.5 + sin(i * step) * 0.1;
		float x2 = -0.5 + cos((i + 1) * step) * 0.1;
		float y2 = -0.5 + sin((i + 1) * step) * 0.1;

		mPoints.push_back({ x, y, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 0.0, 0.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = 0.5 + cos(i * step) * 0.1;
		float y = -0.5 + sin(i * step) * 0.1;
		float x2 = 0.5 + cos((i + 1) * step) * 0.1;
		float y2 = -0.5 + sin((i + 1) * step) * 0.1;

		mPoints.push_back({ x, y, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 1.0, 1.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 0.0, 0.0, 1.0 });
	}

	float alfa = 2;
	step = PI / 8;

	for(int i = 0; i < 16; i++) {
		float x = cos(i * step);
		float y = sin(i * step);
		float x2 = lerp(0.0, x, alfa);
		float y2 = lerp(0.0, y, alfa);

		mPoints.push_back({ x, y, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 0.0, 0.0, 1.0 });
	}

	initShader();

	mMatModel = glGetUniformLocation(mShaderId, "Model");
	mProjModel = glGetUniformLocation(mShaderId, "Projection");

	// projectile
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

void Projectile::Draw() {
	glm::mat4 Model = glm::mat4(1.0);
	glm::mat4 Projection = glm::ortho(0.0f, 800.0f, 0.0f, 800.0f);

	Model = glm::translate(Model, glm::vec3(mTransform.xPos, mTransform.yPos, 0.0f));
	Model = glm::rotate(Model, glm::radians(mRotation), glm::vec3(0.0f, 0.0f, 1.0f));
	Model = glm::scale(Model, glm::vec3(mWidth * mTransform.xScale, mHeight * mTransform.yScale, 1.0f));

	glUniformMatrix4fv(mProjModel, 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(mMatModel, 1, GL_FALSE, glm::value_ptr(Model));

	glBindVertexArray(mVAO);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size() - 32);
	glDrawArrays(GL_LINES, mPoints.size() - 32, mPoints.size());
}

void Projectile::Update(float deltaTime) {
	if (mRotation > 360)
		mRotation = 0;

	mRotation += 3;
	mTransform.xPos += mDirection.x * mSpeed * deltaTime;
	mTransform.yPos += mDirection.y * mSpeed * deltaTime;
}

void Projectile::initShader() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	mShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mShaderId);
}
