#pragma once

#include "ParticleEmitter.h";

class ParticleManager {
public:
	ParticleManager& GetInstance();
	void UpdateParticles();
	void DrawParticles();
	~ParticleManager();
private:
	ParticleManager();
};