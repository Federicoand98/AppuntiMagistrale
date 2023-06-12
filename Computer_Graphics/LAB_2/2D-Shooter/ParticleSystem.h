#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ShaderMaker.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Game.h"
#include <vector>

struct ParticleProps {
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec2 VelocityVariation;
	glm::vec4 ColorBegin;
	glm::vec4 ColorEnd;
	float SizeBegin;
	float SizeEnd;
	float SizeVariation;
	float TimeToLive = 1.0f;
};

class ParticleSystem
{
	public:
		ParticleSystem(class Game* game);

		void Init();
		void Update(float deltaTime);
		void Draw();
		void Emit(const ParticleProps& particleProps);
	private:

		struct Particle {
			glm::vec2 Position;
			glm::vec2 Velocity;
			glm::vec4 ColorBegin;
			glm::vec4 ColorEnd;
			float Rotation;
			float SizeBegin;
			float SizeEnd;
			float TimeToLive = 1.0f;
			float TimeToLiveRemaining = 0.0f;
			bool Active = false;
		};

		std::vector<Particle> m_ParticlePool;
		unsigned int m_PoolIndex = 999;
		unsigned int m_VA;

		void initShader();
};

