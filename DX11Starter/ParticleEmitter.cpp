#include "ParticleEmitter.h"
#include <stdlib.h>     /* srand, rand */

Emitter::Emitter(
	ID3D11Device* device,
	SimpleVertexShader* particleVS,
	SimplePixelShader* particlePS,
	SimpleGeometryShader* particleGS,
	ID3D11ShaderResourceView* texture,
	ID3D11SamplerState* sampler,
	ID3D11BlendState* particleBlendState,
	ID3D11DepthStencilState* particleDepthState
) {
	maxParticles = 10000;

	// emitter properties
	spawnPos = XMFLOAT3(0, 1, 0);
	velocity = XMFLOAT3(1, 1, 1);
	colorTint = XMFLOAT4(1, 1, 1, 1);


	// save
	this->particleVS = particleVS;
	this->particleGS = particleGS;
	this->particlePS = particlePS;

	this->particleTexture = texture;
	this->particleSampler = sampler;

	this->particleBlendState = particleBlendState;
	this->particleDepthState = particleDepthState;

	// fill particles array
	particles = new Particle[maxParticles]();
	for (int i = 0; i < maxParticles; i++) {
		particles[i] = Particle();
		// set default properties
		InitializeParticle(i, spawnPos, velocity, colorTint, colorTint, colorTint, XMFLOAT3(1,1,1));
		//InitializeParticle(i, spawnPos, DirectX::XMFLOAT3((rand() % 10000)*0.0001, i/100.0, (rand()%10000)*0.0001));
		// start with no particles alive
		particles[i].Age = lifetime + 1;
	}
	for (int i = 0; i < maxParticles; i++) {
		particles[i].Age = lifetime + 1;
	}
	// initialize DYNAMIC buffer for particles
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Particle) * maxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialParticleData;
	initialParticleData.pSysMem = particles;

	device->CreateBuffer(&vbd, &initialParticleData, &particleBuffer);
}

Emitter::~Emitter() {
	// deallocate memory
}

void Emitter::SpawnNewParticle() {
	SpawnNewParticle(spawnPos, velocity);
}

void Emitter::SpawnNewParticle(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _velocity) {
	SpawnNewParticle(_position, _velocity, colorTint, colorTint, XMFLOAT4(1, 1, 1, 0), XMFLOAT3(1, 1, 1));
}

void Emitter::SpawnNewParticle(
	DirectX::XMFLOAT3 _position, 
	DirectX::XMFLOAT3 _velocity,
	DirectX::XMFLOAT4 startColor,
	DirectX::XMFLOAT4 midColor,
	DirectX::XMFLOAT4 endColor,
	DirectX::XMFLOAT3 scale) 
{
	// check if there are any available particles
	if (particles[nextParticle].Age < lifetime) {
		std::cout << "max particles exceeded";
		return;
	}
	// particle available, initialize it
	InitializeParticle(nextParticle, _position, _velocity, startColor, midColor, endColor, scale);
	// this particle is at the back of the queue now, go to the next one
	nextParticle = (nextParticle + 1) % maxParticles;
}

DirectX::XMFLOAT3* Emitter::GetSpawnPos() {
	return &spawnPos;
}

void Emitter::InitializeParticle(
	int index, 
	DirectX::XMFLOAT3 _position, 
	DirectX::XMFLOAT3 _velocity, 
	DirectX::XMFLOAT4 startColor, 
	DirectX::XMFLOAT4 midColor, 
	DirectX::XMFLOAT4 endColor, 
	DirectX::XMFLOAT3 scale) 
{
	particles[index].Age = 0;
	particles[index].StartPosition = _position;
	//particles[index].StartColor = particles[index].MidColor= colorTint;
	particles[index].StartColor = startColor;
	particles[index].MidColor = midColor;
	particles[index].EndColor = endColor;
	particles[index].StartVelocity = _velocity;
	particles[index].StartMidEndSizes = scale;
}

void Emitter::Update(float dt) {
	// update particle age
	for (int i = 0; i < maxParticles; i++)
	{
		particles[i].Age += dt;
	}
}

void Emitter::Draw(ID3D11DeviceContext* context, Camera* camera, float deltaTime, float totalTime) {
	// update buffer
	// All particles copied locally - send whole buffer to GPU
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, particles, sizeof(Particle) * maxParticles);

	context->Unmap(particleBuffer, 0);

	// shaders
	particleGS->SetMatrix4x4("world", XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)); // Identity
	particleGS->SetMatrix4x4("view", camera->GetViewMatrix());
	particleGS->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	particleGS->CopyAllBufferData();

	particleVS->SetFloat3("acceleration", DirectX::XMFLOAT3(0,0,0)); // no accelaration
	particleVS->SetFloat("maxLifetime", lifetime);
	particleVS->CopyAllBufferData();

	particlePS->SetSamplerState("trilinear", particleSampler);
	particlePS->SetShaderResourceView("particleTexture", particleTexture);
	particlePS->CopyAllBufferData();

	particleVS->SetShader();
	particlePS->SetShader();
	particleGS->SetShader();

	// Set up states
	float factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(particleBlendState, factor, 0xffffffff);
	context->OMSetDepthStencilState(particleDepthState, 0);

	// Set buffers
	UINT particleStride = sizeof(Particle);
	UINT particleOffset = 0;
	context->IASetVertexBuffers(0, 1, &particleBuffer, &particleStride, &particleOffset);
	//context->IASetVertexBuffers(0, 1, &soBufferRead, &particleStride, &particleOffset);

	// Draw auto - draws based on current stream out buffer
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->Draw(maxParticles,0);

	// Unset Geometry Shader for next frame and reset states
	context->GSSetShader(0, 0, 0);
	context->OMSetBlendState(0, factor, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);

	// Reset topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}