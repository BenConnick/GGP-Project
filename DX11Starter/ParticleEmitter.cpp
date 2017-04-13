#include "ParticleEmitter.h"
#include "Particle.h"

Emitter::Emitter(int maxNumParticles, Material* mat) {
	maxParticles = maxNumParticles;
	material = mat;
	// fill particles array
	particles = new Particle[maxNumParticles]();
	for (int i = 0; i < maxParticles; i++) {
		particles[i] = Particle();
		InitializeParticle(i);
	}
	// initialize DYNAMIC buffer
	//D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Particle) * maxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
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
	particles[index].Position = spawnPos;
	particles[index].Color = colorTint;
	particles[index].Velocity = velocity;
}

void Emitter::Update(float dt) {
	// update particle age
	for (int i = 0; i < maxParticles; i++)
	{
		particles[i].Age += dt;
	}
}

D3D11_BUFFER_DESC* Emitter::GetVertexBuffer() {
	return &vbd;
}