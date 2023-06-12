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

class Background {

public:
	Background(class Game* game);
	virtual ~Background();

	void Init(int numHearths);
	void Draw();
	void Update(float deltaTime);

	void RemoveHearth();
	void AddSkull();

private:
	void initShader(unsigned int* shader);

	class Game* mGame;

	Transform mTransform;
	time_t mSeconds;

	unsigned int mVAO, mVAO_H;
	unsigned int mVBO, mVBO_H;
	unsigned int mShaderId, mShaderId_H;
	unsigned int mMatModel, mMatModel_H;
	unsigned int mProjModel, mProjModel_H;

	int mNumHearths;
	int mNumSkulls;
	int mNTriangles = 20;
	int mVertices = 3 * 2 * mNTriangles;

	std::vector<Point> mPoints;
	std::vector<Point> mPointsHearth;
};

