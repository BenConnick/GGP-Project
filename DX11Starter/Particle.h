#pragma once
#include <DirectXMath.h>

struct Particle
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Velocity;
	DirectX::XMFLOAT4 Color;
	float Scale = 1;
	float Age = 0;
};