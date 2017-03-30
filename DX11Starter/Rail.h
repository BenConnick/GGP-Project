#pragma once
#include <DirectXMath.h>
#include "Mesh.h"
#include "Entity.h"

using namespace DirectX;

class Rail
{
public:
	Rail(Entity* e);
	~Rail();

	//returns the coordinate at which player should put themselves in order to appear attached to this rail
	XMFLOAT3 GetAttachPoint();
	//overload, get an attachment point blank many seconds down rail
	XMFLOAT3 GetAttachPoint(float t);

private:
	Entity* entity;

};

