#pragma once
#include <vector>
#include "Entity.h"

// singleton class in charge of object pooling
// https://en.wikipedia.org/wiki/Object_pool_pattern
class Recycler {
public:
	~Recycler();
	static Recycler& GetInstance();
	void Deactivate(Entity* object);
	void Activate(Entity* object);
	Entity* Reactivate();

	// no constructor or assignment
	Recycler(Recycler const&) = delete;
	void operator=(Recycler const&) = delete;
private:
	//static Recycler instance;
	Recycler();
	std::vector<Entity*> objects;
};