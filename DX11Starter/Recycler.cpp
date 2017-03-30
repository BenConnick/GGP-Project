#include "Recycler.h"

// singleton private constructor
Recycler::Recycler() {
	// do nothing
}

// magic happens here
// http://stackoverflow.com/questions/1008019/c-singleton-design-pattern
Recycler& Recycler::GetInstance() {
	static Recycler instance;	// garunteed to be destroyed
								// initialized on first use
	return instance;
}

Recycler::~Recycler() {
	// clean up pointers
}

// deactivate a specific object
void Recycler::Deactivate(Entity* object) {
	// flag as deactivated
	object->Deactivate();

	// search for object in list
	// this method would be improved with a hash map
	for (int i = 0; i < objects.size(); i++) {
		if (objects[i] == object) {
			return;
		}
	}

	// if not found, add it
	objects.push_back(object);
};

// activate a specific object
void Recycler::Activate(Entity* object) {
	// flag as activated
	object->Activate();
};

// returns a fresh object
Entity* Recycler::Reactivate() {
	for (int i = 0; i < objects.size(); i++) {
		if (!objects[i]->IsActive()) {
			objects[i]->Activate();
			return objects[i];
		}
	}
}

// NEXT: activate a type of object (such as "note block")