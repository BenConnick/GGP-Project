#include "RailSet.h"



RailSet::RailSet(Rail** railArray)
{
	rails = railArray;
}

RailSet::RailSet(Mesh* model, Material* mat, std::vector<Entity*>* entities)
{
	rails = new Rail*[RailSet::railCount];
	for (int i = 0; i < RailSet::railCount; i++) {
		Entity* railEnt = new Entity(model, mat);
		railEnt->SetPosition({ 1 * i - 1.0f,-1.0f,0.0f });
		railEnt->SetScale({ 0.1f, 0.1f, 200.0f });
		railEnt->Activate();
		rails[i] = new Rail(railEnt);
		entities->push_back(railEnt);
	}
}

RailSet::~RailSet()
{
}

Rail* RailSet::GetRail(int index) {
	return rails[index];
}
