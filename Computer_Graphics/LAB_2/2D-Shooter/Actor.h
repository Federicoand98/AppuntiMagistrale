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
#include "Projectile.h"

class Actor {

public:
	Actor(class Game* game, float width, float height);
	virtual ~Actor();

	void Init();
	void Draw();

	void Update(float deltaTime);

	void ProcessKeyboardInput(GLFWwindow* window);

	void Shoot();

	Transform GetTransform() const { return mTransform; }
	float GetWidth() const { return mActorWidth; }
	float GetHeight() const { return mActorHeight; }
	std::vector<class Projectile*> GetProjectiles() { return mProjectiles; }
	
private:
	void initShader();

	class Game* mGame;

	Transform mTransform;
	Vector mDirection;
	float mSpeed;
	float mActorWidth;
	float mActorHeight;
	float mRotation;
	float mShootingCooldown;
	time_t mSeconds;

	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int mShaderId;
	unsigned int mMatModel;
	unsigned int mProjModel;

	int mNTriangles = 30;
	int mVertices = 3 * 2 * mNTriangles;

	std::vector<Point> mPoints;
	std::vector<class Projectile*> mProjectiles ;
};

