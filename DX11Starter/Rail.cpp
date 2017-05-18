#include "Rail.h"



Rail::Rail(XMFLOAT3 pos)
{
	position = pos;
}


Rail::~Rail()
{
}

XMFLOAT3 Rail::GetAttachPoint()
{
	return GetAttachPoint(0);
}
XMFLOAT3 Rail::GetAttachPoint(float t)
{
	XMFLOAT3 pos = position;
	XMFLOAT3 point = { pos.x,pos.y,pos.z+t*20 };
	return point;
}
