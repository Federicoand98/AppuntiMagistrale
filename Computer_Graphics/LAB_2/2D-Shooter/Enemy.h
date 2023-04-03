#pragma once

#include <vector>
#include <random>
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

class Enemy {

public:
	Enemy(class Game* game, float width, float height);
	virtual ~Enemy();

	void Init();
	void Draw();
	// Called every frame to update the Enemy 
	void Update(float deltaTime);

	void ChooseNewDirection();

	Transform GetTransform() const { return mTransform; }
	float GetWidth() const { return mEnemyWidth; }
	float GetHeight() const { return mEnemyHeight; }

private:
	void initShader();

	class Game* mGame;

	Transform mTransform;
	Vector mDirection;
	float mSpeed;
	float mEnemyWidth;
	float mEnemyHeight;
	time_t mSeconds;

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int mShaderId;
	unsigned int mMatModel;
	unsigned int mProjModel;

	int mNTriangles = 30;
	int mVertices = 3 * 2 * mNTriangles;

	std::vector<Point> mPoints;

};

