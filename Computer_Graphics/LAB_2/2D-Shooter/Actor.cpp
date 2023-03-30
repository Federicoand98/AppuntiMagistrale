#include "Actor.h"

#define PI 3.14159265359

Actor::Actor() : mVAO(0), mVBO(0), mShaderId(0) {
	palla = new Point[mVerticesPalla];
}

Actor::~Actor() {
	
}

void Actor::initShader() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	mShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mShaderId);
}

void Actor::Init() {
	/*
	float vertices[] = {
		//posizioni              //Colore
		-0.5f, -0.5f, 0.0f, 0.20f, 0.78f, 0.92f,1.0f, //vertice in basso a sinistra
		0.5f, -0.5f, 0.0f,  0.20f, 0.78f, 0.92f,1.0f, //vertice in basso a destra
		0.0f,  0.5f, 0.0f,  0.20f, 0.78f, 0.92f,1.0f  //vertice in alto
	};

	initShader();

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	*/

	initShader();

	mMatModel = glGetUniformLocation(mShaderId, "Model");
	mProjModel = glGetUniformLocation(mShaderId, "Projection");

	glm::vec4 col_rosso = { 1.0, 0.0, 0.0, 1.0 };
	glm::vec4 col_nero = { 0.0, 0.0, 0.0, 1.0 };
	glm::vec4 col_bottom = { 1.0, 0.8, 0.0, 1.0 };

	float stepA = (2 * PI) / mNTrianglesPalla;
	int comp = 0;

	for(int i = 0; i < mNTrianglesPalla; i++) {
		palla[comp].x = cos((double)i * stepA);
		palla[comp].y = sin((double)i * stepA);
		palla[comp].r = col_rosso.r;
		palla[comp].g = col_rosso.g;
		palla[comp].b = col_rosso.b;
		palla[comp].a = col_rosso.a;

		palla[comp + 1].x = cos((double)i * stepA);
		palla[comp + 1].y = sin((double)i * stepA);
		palla[comp + 1].r = col_rosso.r;
		palla[comp + 1].g = col_rosso.g;
		palla[comp + 1].b = col_rosso.b;
		palla[comp + 1].a = col_rosso.a;

		palla[comp + 2].x = 0.0;
		palla[comp + 2].y = 0.0;
		palla[comp + 2].r = col_bottom.r;
		palla[comp + 2].g = col_bottom.g;
		palla[comp + 2].b = col_bottom.b;
		palla[comp + 2].a = col_bottom.a;

		comp += 3;
	}

	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVerticesPalla * sizeof(Point), &palla[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

double lerp(double a, double b, double amount) {
	return (1 - amount) * a + amount * b;
}

void Actor::Draw() {
	glm::mat4 Model = glm::mat4(1.0);
	double bwidth = 80;
	double bheight = 80;

	float posx = 400.0;
	float posy = 160.0;

	Model = glm::translate(Model, glm::vec3(posx - bwidth / 2, posy + bheight, 0.0f));
	Model = glm::scale(Model, glm::vec3(float(bwidth) / 2, float(bheight)/2, 1.0f));
	glUniformMatrix4fv(mMatModel, 1, GL_FALSE, glm::value_ptr(Model));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, mVerticesPalla / 2);
}

void Actor::Update(float deltaTime) {
	
}
