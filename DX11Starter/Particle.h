#pragma once
#include <DirectXMath.h>

struct Particle
{
	int Type = 0;
	float Age = 0;
	DirectX::XMFLOAT3 StartPosition;
	DirectX::XMFLOAT3 StartVelocity;
	DirectX::XMFLOAT4 StartColor;
	DirectX::XMFLOAT4 MidColor;
	DirectX::XMFLOAT4 EndColor;
	DirectX::XMFLOAT3 StartMidEndSizes;
};