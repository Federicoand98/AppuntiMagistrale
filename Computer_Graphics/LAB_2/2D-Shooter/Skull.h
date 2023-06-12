#pragma once
#include <vector>
#include <time.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ShaderMaker.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Common.h"

class Skull {

public:
	Skull(class Game* game, float width, float height, Vector spawnPosition);
	virtual ~Skull();

	void Init();
	void Draw();

	void Update(float deltaTime);

	Transform GetTransform() const { return mTransform; }
	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }

private:
	void initShader();

	class Game* mGame;

	Transform mTransform;
	Vector mSpawnPosition;
	float mWidth;
	float mHeight;

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int mShaderId;
	unsigned int mMatModel;
	unsigned int mProjModel;

	int mNTriangles = 30;
	int mVertices = 3 * 2 * mNTriangles;

	std::vector<Point> mPoints;
};

