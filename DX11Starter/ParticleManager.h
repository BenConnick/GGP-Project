#pragma once

#include "ParticleEmitter.h";

using namespace DirectX;

class ParticleManager {
public:
	static ParticleManager& GetInstance();
	~ParticleManager();
	void AttachEmitter(Emitter* _emitter);
	void EmitSmallParticle(XMFLOAT3 pos, XMFLOAT3 vel);
	void EmitMedParticle(XMFLOAT3 pos, XMFLOAT3 vel);
	void EmitSkyParticle(XMFLOAT3 pos, XMFLOAT3 vel, XMFLOAT4 color);
	void NoteHitBurst(XMFLOAT3 pos);
	void Update(float dt);
	void SkyColorBurst();
private:
	ParticleManager();
	XMFLOAT3 cyclingColor;
	Emitter* emitter;
	float skyInterval = 1;
	float skyTimer = 0;
	float timer = 0;
};