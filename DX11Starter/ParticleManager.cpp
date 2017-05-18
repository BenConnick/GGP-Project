#include "ParticleManager.h"

ParticleManager& ParticleManager::GetInstance() {
	static ParticleManager instance;	// garunteed to be destroyed
										// initialized on first use
	return instance;
}

// private constructor
ParticleManager::ParticleManager() {

}

ParticleManager::~ParticleManager() {
	delete emitter;
}

// emitter for manager to use with effects
void ParticleManager::AttachEmitter(Emitter* _emitter) {
	emitter = _emitter;
}

void ParticleManager::EmitSmallParticle(XMFLOAT3 pos, XMFLOAT3 vel) {
	emitter->SpawnNewParticle(pos, vel, XMFLOAT4(1,1,1,1),XMFLOAT4(1,1,1,0.5), XMFLOAT4(1,1,1,0),XMFLOAT3(0.1,0.1,0.1));
}

void ParticleManager::EmitMedParticle(XMFLOAT3 pos, XMFLOAT3 vel) {
	emitter->SpawnNewParticle(pos, vel, XMFLOAT4(1, 1, 1, 1), XMFLOAT4(1, 1, 1, 0.5), XMFLOAT4(1, 1, 1, 0), XMFLOAT3(2, 2, 2));
}

void ParticleManager::EmitSkyParticle(XMFLOAT3 pos, XMFLOAT3 vel, XMFLOAT4 color) {
	emitter->SpawnNewParticle(pos, vel, XMFLOAT4(color.x,color.y,color.z,0), color, color, XMFLOAT3(20.1, 20.1, 20.1));
}

void ParticleManager::NoteHitBurst(DirectX::XMFLOAT3 pos) {
	if (emitter == nullptr) return;

	DirectX::XMFLOAT3 vel = DirectX::XMFLOAT3(0, 0, 0);

	for (int i = 0; i < 100; i++) {
		// random velocity
		vel.x = ((rand() % 100) * 0.02 - 1) * 10.0;
		vel.y = ((rand() % 100) * 0.02 - 1) * 10.0;
		vel.z = ((rand() % 100) * 0.02 - 1) * 10.0;

		// shoot particle
		EmitSmallParticle(pos, vel);
	}
}

void ParticleManager::Update(float dt) {
	timer += dt;
	skyTimer += dt;
	if (skyTimer > skyInterval) {
		skyTimer -= skyInterval;
		SkyColorBurst();
	}

	//updateCyclingColor(currentColor);
}

void ParticleManager::SkyColorBurst() {
	if (emitter == nullptr) return;

	XMFLOAT4 color = XMFLOAT4(XMScalarCos(timer), XMScalarSin(timer), XMScalarCos(timer),0.3);
	currentColor = nextColor;
	nextColor = color;

	XMFLOAT3 vel = XMFLOAT3(0, 0, -50);
	XMFLOAT3 pos = XMFLOAT3(0, 0, 50);
	for (int i = 0; i < 200; i++) {
		// random position
		pos.x = (rand()%1000 * 0.001)*100 - 50;
		pos.y = 5;
		pos.z = (rand() % 1000 * 0.001) * 100 + 50;

		// shoot particle
		EmitSkyParticle(pos, vel, color);
	}
}


XMFLOAT4 ParticleManager::GetCyclingColor() {
	float t = timer + 4.5;
	return XMFLOAT4(XMScalarCos(t), XMScalarSin(t), XMScalarCos(t), 0.3);;
}

//"cycling color" is gradually updated until it matches last chosen color
//cycling color is fed directly into the pixel shader
void ParticleManager::updateCyclingColor(XMFLOAT4 color) {
	float speed = 0.05f;
	if (cyclingColor.x < color.x) {
		cyclingColor.x+= min( speed, abs(color.x - cyclingColor.x));
	}
	else if (cyclingColor.x > color.x) {
		cyclingColor.x-= min(speed, abs(color.x - cyclingColor.x));
	}

	if (cyclingColor.y < color.y) {
		cyclingColor.y+= min(speed, abs(color.y - cyclingColor.y));
	}
	else if (cyclingColor.y > color.y) {
		cyclingColor.y-= min(speed, abs(color.y - cyclingColor.y));
	}

	if (cyclingColor.z < color.z) {
		cyclingColor.z+= min(speed, abs(color.z - cyclingColor.z));
	}
	else if (cyclingColor.z > color.z) {
		cyclingColor.z-= min(speed, abs(color.z - cyclingColor.z));
	}

	if (cyclingColor.w < color.w) {
		cyclingColor.w+= min(speed, abs(color.w - cyclingColor.w));
	}
	else if (cyclingColor.w > color.w) {
		cyclingColor.w-= min(speed, abs(color.w - cyclingColor.w));
	}
}