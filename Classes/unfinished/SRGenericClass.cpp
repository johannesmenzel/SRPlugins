#include "SRGenericClass.h"
//#include this and that
//#include <math.h>

namespace SRPlugins
{
namespace SRGenericClass
{

SRGenericClass::SRGenericClass()
{
	// init member variable values so that non empty
	mType = typeOne;
	mVar = 0.0;
}

// Constructor
SRGenericClass::SRGenericClass(int pType, double pVar)
{
	// set members on init
	setClass(pType, pVar);
	// init internal variables
	prev[0] = 0.0;
	prev[1] = 0.0;
	dry[0] = 0.0;
	dry[1] = 0.0;
}

// Destructor
SRGenericClass::~SRGenericClass()
{
}

void SRGenericClass::setType(int pType)
{
	this->mType = pType;
	calcClass(); // optional
}

void SRGenericClass::setVar(double pVar)
{
	this->mVar = pVar;
	calcClass(); // optional
}

void SRGenericClass::setClass(int pType, double pVar)
{
	this->mType = pType;
	this->mVar = pVar;
	calcClass(); // optional
}

// Internal calculations. Example on calculation depending on mType:
void SRGenericClass::calcClass(void)
{
	switch (this->mType)
	{
	case typeOne:
		// do something
		;
		break;
		default:
		break;
	}
	return;
}
} // namespace SRGenericClass
} // namespace SRPlugins