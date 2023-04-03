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
		float x = 0 + cos(i * step) * 1;
		float y = 0 + sin(i * step) * 1;
		float x2 = 0 + cos((i + 1) * step) * 0.2;
		float y2 = 0 + sin((i + 1) * step) * 0.2;

		mPoints.push_back({ x, y, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ x2, y2, 1.0, 0.0, 0.0, 1.0 });
		mPoints.push_back({ 0, 0, 1.0, 1.0, 1.0, 1.0 });
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

	mPoints.push_back({ -0.5, -0.5, 0.0, 0.0, 0.0, 1.0 });
	mPoints.push_back({ -0.25, -0.25, 0.0, 0.0, 0.0, 1.0 });
	mPoints.push_back({ 0.0, -0.5, 0.0, 0.0, 0.0, 1.0 });
	mPoints.push_back({ 0.25, -0.25, 0.0, 0.0, 0.0, 1.0 });
	mPoints.push_back({ -0.5, -0.5, 0.0, 0.0, 0.0, 1.0 });

	float alfa = 2;
	step = PI / 4;

	for(int i = 0; i < 8; i++) {
		float x = cos(i * step);
		float y = sin(i * step);
		float x2 = lerp(0.0, x, alfa);
		float y2 = lerp(0.0, y, alfa);

		mPoints.push_back({ x, y, 0.0, 0.0, 0.0, 0.0 });
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

	// particles
	/*
	glGenVertexArrays(1, &mVAOP);
	glBindVertexArray(mVAOP);
	glGenBuffers(1, &mVBOP);
	glBindBuffer(GL_ARRAY_BUFFER, mVBOP);

	glBindVertexArray(0);
	*/
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
	glDrawArrays(GL_TRIANGLES, 0, mPoints.size());

	// particles
	/*
	for(int i = 0; i < mParticles.size(); i++) {
		mParticles.at(i).xFactor /= mParticles.at(i).drag;
		mParticles.at(i).yFactor /= mParticles.at(i).drag;
		mParticles.at(i).x += mParticles.at(i).xFactor;
		mParticles.at(i).y += mParticles.at(i).yFactor;
		mParticles.at(i).alpha -= 0.05; // reduces life

		float xPos = -1.0f + ((float)mParticles.at(i).x) * 2 / ((float)(800));
		float yPos = -1.0f + ((float)(mParticles.at(i).y)) * 2 / ((float)(800));

		if (mParticles.at(i).alpha <= 0.0) { // particle is dead
			mParticles.erase(mParticles.begin() + i);
		}
		else { // particle is alive, thus update
			Point p;
			p.x = xPos;
			p.y = yPos;
			p.r = mParticles.at(i).color.r;
			p.g = mParticles.at(i).color.g;
			p.b = mParticles.at(i).color.b;
			p.a = mParticles.at(i).alpha;

			mParticlePoints.push_back(p);
		}
	}

	float r = 0.0, g = 0.0, b = 0.0;
	glClearColor(r, g, b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(mVAOP);
	glBindBuffer(GL_ARRAY_BUFFER, mVBOP);
	glBufferData(GL_ARRAY_BUFFER, mParticlePoints.size() * sizeof(Point), &mParticlePoints[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glPointSize(3.0);
	glDrawArrays(GL_POINTS, 0, mParticlePoints.size());
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	*/
}

void Projectile::Update(float deltaTime) {
	if (mRotation > 360)
		mRotation = 0;

	mRotation += 3;
	mTransform.xPos += mDirection.x * mSpeed * deltaTime;
	mTransform.yPos += mDirection.y * mSpeed * deltaTime;

	// adding particles
	/*
	Color rgb = computeRainbow();

	for(int i = 0; i < 10; i++) {
		Particle p;

		p.x = (float)mTransform.xPos;
		p.y = (float)(800 - mTransform.yPos);
		p.alpha = 1.0;
		p.drag = 1.05;
		p.xFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.yFactor = (rand() % 1000 + 1) / 300.0 * (rand() % 2 == 0 ? -1 : 1);
		p.color.r = rgb.r;
		p.color.g = rgb.g;
		p.color.b = rgb.b;

		mParticles.push_back(p);
	}
	*/
}

void Projectile::initShader() {
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"shaders/vertexShader.glsl";
	char* fragmentShader = (char*)"shaders/fragmentShader.glsl";

	mShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(mShaderId);
}

Color Projectile::computeRainbow() {
	float rgb[3] = { 1.0, 0.0, 0.0 };
	int fase = 0;
	int counter = 0;
	float step = 0.1;

	switch (fase) {
	case 0: rgb[1] += step;
		break;
	case 1: rgb[0] -= step;
		break;
	case 2: rgb[2] += step;
		break;
	case 3: rgb[1] -= step;
		break;
	case 4: rgb[0] += step;
		break;
	case 5: rgb[2] -= step;
		break;
	default:
		break;
	}

	counter++;
	if (counter > 1.0 / step) {
		counter = 0;
		fase < 5 ? fase++ : fase = 0;
	}

	return { rgb[0], rgb[1], rgb[2] };
}
