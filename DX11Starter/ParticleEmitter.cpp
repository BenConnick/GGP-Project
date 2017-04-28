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
	maxParticles = 100;

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
		InitializeParticle(i);
		// start with no particles alive
		//particles[i].Age = lifetime;
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
	// check if there are any available particles
	if (particles[nextParticle].Age < lifetime) return;
	// particle available, initialize it
	InitializeParticle(nextParticle);
	// this particle is at the back of the queue now, go to the next one
	nextParticle++;
}

DirectX::XMFLOAT3* Emitter::GetSpawnPos() {
	return &spawnPos;
}

void Emitter::InitializeParticle(int index) {
	particles[index].Age = 0;
	particles[index].StartPosition = spawnPos;
	particles[index].StartColor = particles[index].MidColor = particles[index].EndColor = colorTint;
	velocity = DirectX::XMFLOAT3((rand() % 100)*0.01, (rand() % 100)*0.01, (rand() % 100)*0.01);
	particles[index].StartVelocity = velocity;
	particles[index].StartMidEndSizes = XMFLOAT3(1,1,1);
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
	context->Draw(100,0);

	// Unset Geometry Shader for next frame and reset states
	context->GSSetShader(0, 0, 0);
	context->OMSetBlendState(0, factor, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);

	// Reset topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}