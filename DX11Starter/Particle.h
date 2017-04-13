#pragma once
#include <DirectXMath.h>

struct Particle
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Velocity;
	DirectX::XMFLOAT4 Color;
	float scale = 1;
	float lifeTime = 0;
};