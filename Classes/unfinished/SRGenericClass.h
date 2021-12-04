//  SRGenericClass.h

// Implementation:
//   Note that you might need a class for each channel
//   (so 2 for stereo processing.)
//
// Header:
//   private:
//     Impementation as object:
//       SRGenericClass name;
//     Implementation as pointer:
//       SRGenericClass *name = new SRGenericClass();
//
// Class: Constructor, Reset()
//     Implementation as object:
//       name.setClass(pType, pVar1, pVar2, pVar3);
//     Implementation as pointer:
//       name->setClass(pType, pVar1, pVar2, pVar3);
//
// Class: ProcessDoubleReplacing()
//   Per sample and channel:
//     Implementation as object:
//       *out1 = name.process(*in1);
//     Implementation as pointer:
//       *out1 = name->process(*in1);

#ifndef SRGenericClass_h
#define SRGenericClass_h

// #include this and that
#include "../Utils/SRHelpers.h" //optional

namespace SRPlugins
{
namespace SRGenericClassNamespace
{

// If type definitions of type int needed:
enum
{
	typeOne = 0,
	numTypes
	// ...
};

class SRGenericClass
{
  public:
	// constructor
	SRGenericClass();
	// class initializer
	SRGenericClass(
		int pType,
		double pVar);
	// destructor
	~SRGenericClass(); // destructor

	// public functions that need to be accessed from outside
	void setType(int pType);
	void setVar(double pVar); // create these for every member
	void getVar() { return mVar; }

	void setClass(
		int pType,
		double pVar);
	// inline process functions, if needed
	void process(double &in);				// for mono
	void process(double &in1, double &in2); // for stereo

  protected:
	// Protected functions that do internal calculations and that are called from other funcions
	void calcClass(void);

	// Internal member and internal variables
	int mType;
	// member variables
	double mVar;
	// internal variables
	double prev[2];
	double dry[2];
}; // end of class

inline void SRGenericClass::process(double &in)
{
	// create dry samples
	dry[0] = in;

	// process
	// ...

	prev[0] = dry[0];
}

inline void SRGenericClass::process(double &in1, double &in2)
{
	// create dry samples
	dry[0] = in1;
	dry[1] = in2;

	// process
	// ...

	prev[0] = dry[0];
	prev[1] = dry[1];
}

} // namespace SRGenericClassNamespace
} // namespace SRPlugins
#endif // SRGenericClass_h