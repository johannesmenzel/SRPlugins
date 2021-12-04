//  SRComponents.h

// Implementation:
//   Note that you might need a class for each channel
//   (so 2 for stereo processing.)
//
// Header:
//   private:
//     Impementation as object:
//       SRTube name;
//     Implementation as pointer:
//       SRTube *name = new SRTube();
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

#ifndef SRComponents_h
#define SRComponents_h

#include "../Utils/SRHelpers.h"
// #include this and that

namespace SRPlugins
{
namespace SRComponents
{

// If type definitions of type int needed:
enum
{
	type12AX7 = 0,
	numTypes
	// ...
};

class SRTube
{
  public:
	// constructor
	SRTube();
	// class initializer
	SRTube(
		int pType,
		double pDriveDb,
		double pAmountNormalized,
		double pHarmonicsNormalized,
		bool pPositiveSide,
		double pSkewNormalized,
		double pWet);
	// destructor
	~SRTube(); // destructor

	// public functions that need to be accessed from outside
	void setType(int pType);
	void setDrive(double pDriveDb);
	void setAmount(double pAmountNormalized);
	void setHarmonics(double pHarmonicsNormalized);
	void setPositive(bool pPositive);
	void setSkew(double pSkewNormalized);
	void setWet(double pWetNormalized);

	void setTube(
		int pType,
		double pDriveDb,
		double pAmountNormalized,
		double pHarmonicsNormalized,
		double pPositive,
		double pSkewNormalized,
		double pWetNormalized);
	// inline process function, if needed
	void process(double &in1, double &in2);

  protected:
	// Protected functions that do internal calculations and that are called from other funcions
	void calcTube(void);

	// Internal member and internal variables
	int mType;
	// member variables
	double mDriveNormalized;
	double mAmountNormalized;
	double mAmount;
	double mHarmonicsNormalized;
	bool mPositive; // if aiming for even harmonics, the positive side of the envelope will be affected if true, otherwise the negative side
	double mSkewNormalized;
	double mWetNormalized;
	// internal variables
	double in1PrevSample;
	double in2PrevSample;
	double in1Dry;
	double in2Dry;

}; // end of class

inline void SRTube::process(double &in1, double &in2)
{
	// create driven dry samples
	in1Dry = in1;
	in2Dry = in2;

	// process
	// ..

  in1 = in1PrevSample;
  in2 = in2PrevSample;
}

} // namespace SRComponents
} // namespace SRPlugins

#endif // SRComponents_h