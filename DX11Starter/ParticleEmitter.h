#pragma once
#include <vector>
#include <DirectXMath.h>
#include <iostream>
#include "Particle.h"
#include "Material.h"
#include "Camera.h"

class Emitter {
public:
	Emitter(
		ID3D11Device* device,
		SimpleVertexShader* particleVS,
		SimplePixelShader* particlePS,
		SimpleGeometryShader* particleGS,
		ID3D11ShaderResourceView* texture,
		ID3D11SamplerState* sampler,
		ID3D11BlendState* particleBlendState,
		ID3D11DepthStencilState* particleDepthState
	);
	~Emitter();
	void SpawnNewParticle();
	void SpawnNewParticle(XMFLOAT3 pos, XMFLOAT3 vel);
	void SpawnNewParticle(
		DirectX::XMFLOAT3 _position,
		DirectX::XMFLOAT3 _velocity,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 midColor,
		DirectX::XMFLOAT4 endColor,
		DirectX::XMFLOAT3 scale);
	DirectX::XMFLOAT3* GetSpawnPos();
	void InitializeParticle(
		int index,
		DirectX::XMFLOAT3 _position,
		DirectX::XMFLOAT3 _velocity,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 midColor,
		DirectX::XMFLOAT4 endColor,
		DirectX::XMFLOAT3 scale);
	void Update(float dt);

	void Draw(ID3D11DeviceContext* context, Camera* camera, float deltaTime, float totalTime);

private:

	// particle resources
	SimpleVertexShader* particleVS;
	SimplePixelShader* particlePS;
	SimpleGeometryShader* particleGS;

	ID3D11ShaderResourceView* particleTexture;
	ID3D11SamplerState* particleSampler;
	ID3D11BlendState* particleBlendState;
	ID3D11DepthStencilState* particleDepthState;

	// buffer matrix for ParticleGS cbuffer
	ID3D11Buffer* stuff;

	int numLiving = 0;
	int nextParticle = 0;
	DirectX::XMFLOAT3 spawnPos;
	DirectX::XMFLOAT3 velocity;
	int maxParticles;
	Material* material;
	// circular buffer of particles
	Particle* particles;
	float lifetime = 5;
	DirectX::XMFLOAT4 colorTint;
	D3D11_BUFFER_DESC vbd;
	ID3D11Buffer* particleBuffer;
};