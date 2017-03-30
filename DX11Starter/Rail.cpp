#include "Rail.h"



Rail::Rail(Entity* e)
{
	entity = e;
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
	XMFLOAT3 pos = entity->GetPosition();
	XMFLOAT3 point = { pos.x,pos.y+t,pos.z };
	return point;
}
