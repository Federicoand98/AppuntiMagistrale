#include "Background.h"

#include "Game.h"

Background::Background(Game* game) {
	mTransform = {
		400.0f,
		400.0f,
		1,
		1
	};
}

Background::~Background() {
	mPoints.clear();

	glBindVertexArray(0);

	glDeleteProgram(mShaderId);
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

void Background::Init(int numHearths) {
	// background rect
	mPoints.push_back({ 1, 1, 0.77, 0.68, 1, 1 });
	mPoints.push_back({ -1, 1, 0.77, 0.68, 1, 1 });
	mPoints.push_back({ -1, -1, 0.39, 0.43, 1, 1 });
	mPoints.push_back({ -1, -1, 0.39, 0.43, 1, 1 });
	mPoints.push_back({ 1, -1, 0.39, 0.43, 1, 1 });
	mPoints.push_back({ 1, 1, 0.77, 0.68, 1, 1 });

	initShader(&mShaderId);

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

// TODO: fix width and height with game's properties
void Background::Draw() {
	glm::mat4 Model = glm::mat4(1.0);
	glm::mat4 Projection = glm::ortho(0.0f, 800.0f, 0.0f, 800.0f);

	Model = glm::translate(Model, glm::vec3(mTransform.xPos, mTransform.yPos, 0.0f));
	Model = glm::scale(Model, glm::vec3(800 * mTransform.xScale, 800 * mTransform.yScale, 1.0f));

	glUseProgram(mShaderId);
	glUniformMatrix4fv(mProjModel, 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(mMatModel, 1, GL_FALSE, glm::value_ptr(Model));

	glBindVertexArray(mVAO);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size());
}

void Background::Update(float deltaTime) {
	
}

void Background::initShader(unsigned int* shader) {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	*shader = ShaderMaker::createProgram(vertexShader, fragmentShader);
}
