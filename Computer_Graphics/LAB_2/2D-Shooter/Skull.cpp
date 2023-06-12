#include "Skull.h"

Skull::Skull(Game* game, float width, float height, Vector spawnPosition) {
	mGame = game;

	this->mTransform = {
		spawnPosition.x,
		spawnPosition.y,
		1.0f,
		1.0f
	};

	this->mWidth = width;
	this->mHeight = height;
}

Skull::~Skull() {
	mPoints.clear();

	glBindVertexArray(0);

	glDeleteProgram(mShaderId);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void Skull::Init() {
	float step = (1 * PI) / mNTriangles;

	for (double i = 0; i < mNTriangles; i += step) {
		float x = cos(i * step) ;
		float y = sin(i * step);
		float x2 = cos((i + 1) * step);
		float y2 = sin((i + 1) * step);

		mPoints.push_back({ x, y, 0.82, 0.82, 0.82, 1.0 });
		mPoints.push_back({ x2, y2, 0.82, 0.82, 0.82, 1.0 });
		mPoints.push_back({ 0, 0, 0.82, 0.82, 0.82, 1.0 });
	}

	step = (2 * PI) / mNTriangles;

	for (double i = 0; i < mNTriangles; i += step) {
		float x = -0.4 + cos(i * step) * 0.2;
		float y = 0.4 + sin(i * step) * 0.2;
		float x2 = -0.4 + cos((i + 1) * step) * 0.2;
		float y2 = 0.4 + sin((i + 1) * step) * 0.2;

		mPoints.push_back({ x, y, 0.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ x2, y2, 0.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ -0.4, 0.4, 0.0, 0.0, 0.0, 1.0 });
	}

	for (double i = 0; i < mNTriangles; i += step) {
		float x = 0.4 + cos(i * step) * 0.2;
		float y = 0.4 + sin(i * step) * 0.2;
		float x2 = 0.4 + cos((i + 1) * step) * 0.2;
		float y2 = 0.4 + sin((i + 1) * step) * 0.2;

		mPoints.push_back({ x, y, 0.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ x2, y2, 0.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ 0.4, 0.4, 0.0, 0.0, 0.0, 1.0 });
	}

	mPoints.push_back({ 1, 0, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ -1, 0, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ -1, -0.4, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ -1, -0.4, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ 1, -0.4, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ 1, 0, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ -0.6, -0.4, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ -0.6, -1, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ 0.6, -1, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ 0.6, -1, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ 0.6, -0.4, 0.82, 0.82, 0.82, 1.0 });
	mPoints.push_back({ -0.6, -0.4, 0.82, 0.82, 0.82, 1.0 });
	// nose
	mPoints.push_back({ 0, 0.2, 0.0, 0.0, 0.0, 1.0 });
	mPoints.push_back({ -0.2, -0.2, 0.0, 0.0, 0.0, 1.0 });
	mPoints.push_back({ 0.2, -0.2, 0.0, 0.0, 0.0, 1.0 });
	// teeth
	float x = -0.4;
	for(int i = 0; i < 5; i++) {
		mPoints.push_back({ x, -1, 0, 0, 0, 1 });
		mPoints.push_back({ x, -0.4, 0, 0, 0, 1 });
		x += 0.2;
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

void Skull::Draw() {
	glm::mat4 Model = glm::mat4(1.0);
	glm::mat4 Projection = glm::ortho(0.0f, 800.0f, 0.0f, 800.0f);

	Model = glm::translate(Model, glm::vec3(mTransform.xPos, mTransform.yPos, 0.0f));
	Model = glm::scale(Model, glm::vec3(mWidth * mTransform.xScale, mHeight * mTransform.yScale, 1.0f));

	glUniformMatrix4fv(mProjModel, 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(mMatModel, 1, GL_FALSE, glm::value_ptr(Model));

	glBindVertexArray(mVAO);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size() - 10);
	glDrawArrays(GL_LINES, mPoints.size() - 10, mPoints.size());
}

void Skull::Update(float deltaTime) {}

void Skull::initShader() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	mShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mShaderId);
}
