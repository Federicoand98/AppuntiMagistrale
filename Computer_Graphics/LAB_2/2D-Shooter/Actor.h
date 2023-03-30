#pragma once

#include <vector>
#include <GL/glew.h>
#include "ShaderMaker.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Point {
	float x;
	float y;
	float r;
	float g;
	float b;
	float a;
};

class Actor {

public:
	Actor();
	virtual ~Actor();

	void Init();
	void Draw();
	// Called every frame to update the Actor
	void Update(float deltaTime);

private:
	void initShader();

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int mShaderId;
	unsigned int mMatModel;
	unsigned int mProjModel;

	int mNTrianglesPalla = 30;
	int mVerticesPalla = 3 * 2 * mNTrianglesPalla;

	Point* palla;
};

