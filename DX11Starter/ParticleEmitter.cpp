#include "ParticleEmitter.h"
#include "Particle.h"

Emitter::Emitter(int maxNumParticles, Material* mat) {
	maxParticles = maxNumParticles;
	material = mat;
	// fill particles array
	particles = new Particle[maxNumParticles]();
	for (int i = 0; i < maxParticles; i++) {
		particles[i] = Particle();
		particles[i].Position = spawnPos;
		particles[i].Color = colorTint;
	}
}

Emitter::~Emitter() {
	// deallocate memory
}

void Emitter::SpawnNewParticle() {
	particle
}

DirectX::XMFLOAT3* Emitter::GetSpawnPos() {
	return &spawnPos;
}