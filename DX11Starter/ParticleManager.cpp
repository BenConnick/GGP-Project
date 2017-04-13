#include "ParticleManager.h"

ParticleManager::ParticleManager() {

}

ParticleManager::~ParticleManager() {

}

void ParticleManager::UpdateParticles() {

}

void ParticleManager::DrawParticles() {

}

ParticleManager& ParticleManager::GetInstance() {
	static ParticleManager instance;	// garunteed to be destroyed
										// initialized on first use
	return instance;
}