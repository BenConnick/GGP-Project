#include "RailSet.h"



RailSet::RailSet(Rail** railArray)
{
	rails = railArray;
}


RailSet::~RailSet()
{
}

Rail* RailSet::GetRail(int index) {
	return rails[index];
}
