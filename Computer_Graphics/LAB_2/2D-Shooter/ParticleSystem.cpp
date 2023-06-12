#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(Game* game) {
	m_ParticlePool.resize(1000);
}

void ParticleSystem::Init() {
	
}

void ParticleSystem::Update(float deltaTime) {

	for(Particle& particle : m_ParticlePool) {
		if(!particle.Active)
			continue;

		if(particle.TimeToLiveRemaining <= 0.0f) {
			particle.Active = false;
			continue;
		}

		particle.TimeToLiveRemaining -= deltaTime;
		particle.Position += particle.Velocity * deltaTime;
		particle.Rotation += 0.01f * deltaTime;
	}
}

void ParticleSystem::Draw() {
	if(!m_VA) {
		float vertices[] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		unsigned int VB, IB;

		glGenVertexArrays(1, &m_VA);
		glBindVertexArray(m_VA);

		glGenBuffers(1, &VB);
		glBindBuffer(GL_ARRAY_BUFFER, VB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		unsigned int indices[] = {
			0, 1, 2, 2, 3, 0
		};

		glGenBuffers(1, &IB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	}
}

void ParticleSystem::Emit(const ParticleProps& particleProps) {
	
}

void ParticleSystem::initShader() {
	
}
