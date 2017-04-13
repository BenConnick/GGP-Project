#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Particle.h"
#include "Material.h"

class Emitter {
public:
	Emitter(int maxNumParticles, Material* mat);
	~Emitter();
	void SpawnNewParticle();
	DirectX::XMFLOAT3* GetSpawnPos();
private:
	DirectX::XMFLOAT3 spawnPos;
	int maxParticles;
	Material* material;
	int nextParticle = 0;
	// circular buffer of particles
	Particle* particles;
	float lifetime = 1;
	DirectX::XMFLOAT4 colorTint;
};