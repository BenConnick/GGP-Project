#pragma once
#include <vector>
#include "Entity.h"

class Recycler {
public:
	~Recycler();
	static Recycler& GetInstance();
	void Deactivate(Entity* object);
	void Activate(Entity* object);

	// no constructor or assignment
	Recycler(Recycler const&) = delete;
	void operator=(Recycler const&) = delete;
private:
	//static Recycler instance;
	Recycler();
	std::vector<Entity*> objects;
};