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

class Projectile {
public:
	Projectile(class Game* game, float width, float height, Vector spawnPosition, int direction);
	virtual ~Projectile();

	void Init();
	void Draw();
	// Called every frame to update the Actor
	void Update(float deltaTime);

	Transform GetTransform() const { return mTransform; }
	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }
	float GetTime() const { return mSeconds; }
	float GetTimeToLive() const { return mTimeToLive; }

private:
	void initShader();
	Color computeRainbow();

	class Game* mGame;

	Transform mTransform;
	Vector mDirection;
	Vector mSpawnPosition;
	float mSpeed;
	float mWidth;
	float mHeight;
	float mRotation;
	float mSeconds;
	float mTimeToLive;

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int mVAOP;
	unsigned int mVBOP;
	unsigned int mShaderId;
	unsigned int mShaderIdP;
	unsigned int mMatModel;
	unsigned int mProjModel;

	int mNTriangles = 30;
	int mVertices = 3 * 2 * mNTriangles;

	std::vector<Point> mPoints;
	std::vector<Particle> mParticles;
	std::vector<Point> mParticlePoints;
};

