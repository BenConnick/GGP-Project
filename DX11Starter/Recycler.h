#pragma once
#include <vector>
#include "Entity.h"
#include "MusicNode.h"

// singleton class in charge of object pooling
// https://en.wikipedia.org/wiki/Object_pool_pattern
class Recycler {
public:
	~Recycler();
	static Recycler& GetInstance();
	void Deactivate(Entity* object);
	void Activate(Entity* object);
	Entity* Reactivate();

	//MusicNode based overload functions
	void Deactivate(MusicNode* object);
	void Activate(MusicNode* object);
	MusicNode* ReactivateNode();

	// no constructor or assignment
	Recycler(Recycler const&) = delete;
	void operator=(Recycler const&) = delete;
private:
	//static Recycler instance;
	Recycler();
	std::vector<Entity*> objects;

	std::vector<MusicNode*> nodes;
};