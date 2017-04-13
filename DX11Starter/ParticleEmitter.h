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
	void InitializeParticle(int index);
	void Update(float dt);
	D3D11_BUFFER_DESC* GetVertexBuffer();
private:
	int nextParticle = 0;
	DirectX::XMFLOAT3 spawnPos;
	DirectX::XMFLOAT3 velocity;
	int maxParticles;
	Material* material;
	// circular buffer of particles
	Particle* particles;
	float lifetime = 1;
	DirectX::XMFLOAT4 colorTint;
	D3D11_BUFFER_DESC vbd;
};